/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file implements a the USB Communications Device Class - Abstract
 * Control Model (CDC-ACM) as defined in CDC PSTN subclass 1.2.
 *
 * The device's unique id is used as the USB serial number string.
 */



/*
*
*	Changed by: Jiří Veverka (xvever12)
*	Added new (virtual) interface
*
*/
#include "general.h"
#include "cdcacm.h"
#include "usbuart.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/usart.h>
#include <flash/flash_eeprom.h> //Added by xvever12 - for reading serial number from flash
#include <stdlib.h>

#define USB_DRIVER      st_usbfs_v1_usb_driver
#define USB_IRQ	        NVIC_USB_LP_CAN_RX0_IRQ
#define USB_ISR	        usb_lp_can_rx0_isr

#define IRQ_PRI_USB		(2 << 4)

usbd_device *usbdev;

static int configured;

static void cdcacm_set_modem_state(usbd_device *dev, int iface, bool dsr, bool dcd);

static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xEF,		/* Miscellaneous Device */
	.bDeviceSubClass = 2,		/* Common Class */
	.bDeviceProtocol = 1,		/* Interface Association */
	.bMaxPacketSize0 = 64,
	.idVendor = 0x1D50,
	.idProduct = 0x6018,
	.bcdDevice = 0x0100,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/* Serial ACM interfaces */
/* IN endpoints (starting 0x8...) */
static const struct usb_endpoint_descriptor uart_comm_endp1[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x86,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16, //Changed to 16 from 8
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor uart_comm_endp2[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor uart_comm_endp3[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x84,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

/*
	Added by xvever12
	Virtual - communication usb endpoint
*/
static const struct usb_endpoint_descriptor virtual_comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x88,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};
/* -------------end of section------------------*/

/* OUT endpoints (starting 0x0....) */
static const struct usb_endpoint_descriptor uart_data_endp1[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x05,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE / 2,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x85,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE,
	.bInterval = 1,
}};

static const struct usb_endpoint_descriptor uart_data_endp2[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE / 2,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE,
	.bInterval = 1,
}};

static const struct usb_endpoint_descriptor uart_data_endp3[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x03,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE / 2,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE,
	.bInterval = 1,
}};

/*
	Added by xvever12
	Virtual - data usb endpoint
*/
static const struct usb_endpoint_descriptor virtual_data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x07,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE / 2,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x87,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDCACM_PACKET_SIZE,
	.bInterval = 1,
}};
/*-------------end of section ---------------*/

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) uart_cdcacm_functional_descriptors1 = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 5,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 2, /* SET_LINE_CODING supported */
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 4,
		.bSubordinateInterface0 = 5,
	 }
};


static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) uart_cdcacm_functional_descriptors2 = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 2, /* SET_LINE_CODING supported*/
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 }
};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) uart_cdcacm_functional_descriptors3 = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 3,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 2, /* SET_LINE_CODING supported*/
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 2,
		.bSubordinateInterface0 = 3,
	 }
};


/*
	Added by xvever12
	Virtual - composite functional usb descriptor
*/
static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) uart_cdcacm_functional_descriptors_virtual = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 7,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 2, /* SET_LINE_CODING supported*/
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 6,
		.bSubordinateInterface0 = 7,
	 }
};
/*-----------------end of section---------------*/

static const struct usb_interface_descriptor uart_comm_iface1[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 4,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	//.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_NONE,
	.iInterface = 5,

	.endpoint = uart_comm_endp1,

	.extra = &uart_cdcacm_functional_descriptors1,
	.extralen = sizeof(uart_cdcacm_functional_descriptors1)
}};


static const struct usb_interface_descriptor uart_comm_iface2[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	//.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_NONE,
	.iInterface = 5,

	.endpoint = uart_comm_endp2,

	.extra = &uart_cdcacm_functional_descriptors2,
	.extralen = sizeof(uart_cdcacm_functional_descriptors2)
}};



static const struct usb_interface_descriptor uart_comm_iface3[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 2,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	//.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_NONE,
	.iInterface = 5,

	.endpoint = uart_comm_endp3,

	.extra = &uart_cdcacm_functional_descriptors3,
	.extralen = sizeof(uart_cdcacm_functional_descriptors3)
}};

/*
	Added by xvever12
	Virtual - composite communication usb interface
*/
static const struct usb_interface_descriptor virtual_comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 6,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	//.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_NONE,
	.iInterface = 5,

	.endpoint = virtual_comm_endp,

	.extra = &uart_cdcacm_functional_descriptors_virtual,

	.extralen = sizeof(uart_cdcacm_functional_descriptors_virtual),
}};
/*----------------end of section-----------------------*/

static const struct usb_interface_descriptor uart_data_iface1[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 5,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = uart_data_endp1,
}};


static const struct usb_interface_descriptor uart_data_iface2[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = uart_data_endp2,
}};


static const struct usb_interface_descriptor uart_data_iface3[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 3,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = uart_data_endp3,
}};

/*
	Added by xvever12
	Virtual - composite data usb interface
*/
static const struct usb_interface_descriptor virtual_data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 7,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = virtual_data_endp,
}};
/*----------------end of section--------------*/

static const struct usb_iface_assoc_descriptor uart_assoc1 = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 4,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	//.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.bFunctionProtocol = USB_CDC_PROTOCOL_NONE,
	.iFunction = 0,
};

static const struct usb_iface_assoc_descriptor uart_assoc2 = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 0,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	//.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.bFunctionProtocol = USB_CDC_PROTOCOL_NONE,
	.iFunction = 0,
};

static const struct usb_iface_assoc_descriptor uart_assoc3 = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 2,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	//.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.bFunctionProtocol = USB_CDC_PROTOCOL_NONE,
	.iFunction = 0,
};

/*
	Added by xvever12
	Virtual - associative USB descriptor
*/
static const struct usb_iface_assoc_descriptor virt_uart_assoc = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 6,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	//.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.bFunctionProtocol = USB_CDC_PROTOCOL_NONE,
	.iFunction = 0,
};
/*------------------end of section-----------------*/


static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.iface_assoc = &uart_assoc2,
	.altsetting = uart_comm_iface2,
}, {
	.num_altsetting = 1,
	.altsetting = uart_data_iface2,
}, {
	.num_altsetting = 1,
	.iface_assoc = &uart_assoc3,
	.altsetting = uart_comm_iface3,
}, {
	.num_altsetting = 1,
	.altsetting = uart_data_iface3,
}, {
	.num_altsetting = 1,
	.iface_assoc = &uart_assoc1,
	.altsetting = uart_comm_iface1,
}, {
	.num_altsetting = 1,
	.altsetting = uart_data_iface1,
},{
	.num_altsetting = 1,					//Added by xvever12
	.iface_assoc = &virt_uart_assoc,		//this block adds new virtual interface
	.altsetting = virtual_comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = virtual_data_iface,		// end of section
}
};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = sizeof(ifaces)/sizeof(ifaces[0]),
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0xFA, //Added by xvever12: Max 500mA

	.interface = ifaces,
};

/*
	Added by xvever12
	String containing board serial number
*/
char board_no_string[30] = "";
/*----------end of section-----------*/

/* Changed by xvever12 */
static const char *usb_strings[] = {
	"Jiri Veverka",	 //Changed to Jiri Veverka from Black Sphere Technologies
	BOARD_IDENT,
	board_no_string,	//Changed serial_no to board_no_string
	"JaQT Port 0",		//Changed port name
	"JaQT Port 1",		//changed port name
};
/*----------end of section-----------*/

static enum usbd_request_return_codes cdcacm_control_request(usbd_device *dev,
		struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
		void (**complete)(usbd_device *dev, struct usb_setup_data *req))
{
	(void)dev;
	(void)complete;
	(void)buf;
	(void)len;
	//TODO: Check changed line state (modem)
	switch(req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		cdcacm_set_modem_state(dev, req->wIndex, true, true);
		/* Ignore since is not for GDB interface */
        return USBD_REQ_HANDLED;

	case USB_CDC_REQ_SET_LINE_CODING:
		if(*len < sizeof(struct usb_cdc_line_coding))
			return USBD_REQ_NOTSUPP;

		switch(req->wIndex) {
		case 0:
			usbuart_set_line_coding((struct usb_cdc_line_coding*)*buf, USART2);
			return USBD_REQ_HANDLED;
		case 2:
			usbuart_set_line_coding((struct usb_cdc_line_coding*)*buf, USART3);
			return USBD_REQ_HANDLED;
		case 4:
			usbuart_set_line_coding((struct usb_cdc_line_coding*)*buf, USART1);
			return USBD_REQ_HANDLED;
		case 6:
			//added by xvever12: We do not need to set line coding as we use this interface to only communicate with MCU/shell
			return USBD_REQ_HANDLED;
		default:
			return USBD_REQ_NOTSUPP;
		}
	}

	return USBD_REQ_NOTSUPP;
}

int cdcacm_get_config(void)
{
	return configured;
}

static void cdcacm_set_modem_state(usbd_device *dev, int iface, bool dsr, bool dcd)
{
	char buf[10];
	struct usb_cdc_notification *notif = (void*)buf;
	/* We echo signals back to host as notification */
	notif->bmRequestType = 0xA1;
	notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
	notif->wValue = 0;
	notif->wIndex = iface;
	notif->wLength = 2;
	buf[8] = (dsr ? 2 : 0) | (dcd ? 1 : 0);
	buf[9] = 0;
	usbd_ep_write_packet(dev, 0x82 + iface, buf, 10);
}

static void cdcacm_set_config(usbd_device *dev, uint16_t wValue)
{
	configured = wValue;

	/* Serial interface */
	usbd_ep_setup(dev, 0x01, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE / 2, usbuart2_usb_out_cb);
	usbd_ep_setup(dev, 0x81, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE, usbuart_usb_in_cb);
	usbd_ep_setup(dev, 0x82, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL); //changed 8 to 16

	usbd_ep_setup(dev, 0x03, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE / 2, usbuart3_usb_out_cb);
	usbd_ep_setup(dev, 0x83, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE, usbuart_usb_in_cb);
	usbd_ep_setup(dev, 0x84, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_ep_setup(dev, 0x05, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE / 2, usbuart1_usb_out_cb);
	usbd_ep_setup(dev, 0x85, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE, usbuart_usb_in_cb);
	usbd_ep_setup(dev, 0x86, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

/*
	Added by xvever12
	mapping virtual endpoint to physical interface
*/
	usbd_ep_setup(dev, 0x07, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE / 2, usbuartvirt_usb_out_cb);
	usbd_ep_setup(dev, 0x87, USB_ENDPOINT_ATTR_BULK,
	              CDCACM_PACKET_SIZE, NULL);
	usbd_ep_setup(dev, 0x88, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);
/*---------------end of section-------------*/

	usbd_register_control_callback(dev,
			USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
			USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
			cdcacm_control_request);

	/* Notify the host that DCD is asserted.
	 * Allows the use of /dev/tty* devices on *BSD/MacOS
	 */
	cdcacm_set_modem_state(dev, 0, true, true);
	cdcacm_set_modem_state(dev, 2, true, true);
	cdcacm_set_modem_state(dev, 4, true, true);
	cdcacm_set_modem_state(dev, 6, true, true); //Added by xvevre12 - set modem state for virtual interface
}

/* We need a special large control buffer for this device: */
uint8_t usbd_control_buffer[256];


void cdcacm_init(void)
{
	void exti15_10_isr(void);
	/* Added by xvever12: Initialize board serial number string */
	sprintf(board_no_string, "JaQT Board No.: %d", *(flash_read_board_no()));
	/*-----------------end of section-------------------*/

	usbdev = usbd_init(&USB_DRIVER, &dev, &config, usb_strings,
					  sizeof(usb_strings)/sizeof(char *),
					  usbd_control_buffer, sizeof(usbd_control_buffer));
	

	usbd_register_set_config_callback(usbdev, cdcacm_set_config);

	nvic_set_priority(USB_IRQ, IRQ_PRI_USB);
	nvic_enable_irq(USB_IRQ);
}

void USB_ISR(void)
{
	usbd_poll(usbdev);
}

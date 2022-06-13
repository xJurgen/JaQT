/* *****************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 LeafLabs LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * ****************************************************************************/


/**
 *  @file usb_descriptor.c
 *
 *  @brief aka application descriptor; big static struct and callbacks for sending
 *  the descriptor.
 *
 */


#include "usb_descriptor.h"

u8 u8_usbDeviceDescriptorDFU[18] = {
    0x12,   /* bLength */
    0x01,   /* bDescriptorType */
    0x00,   /* bcdUSB, version 1.00 */
    0x01,
    0x00,   /* bDeviceClass : See interface */
    0x00,   /* bDeviceSubClass : See interface*/
    0x00,   /* bDeviceProtocol : See interface */
    bMaxPacketSize, /* bMaxPacketSize0 0x40 = 64 */
    VEND_ID0,   /* idVendor     (0110) */
    VEND_ID1,

    PROD_ID0,   /* idProduct (0x1001 or 1002) */
    PROD_ID1,

    0x01,   /* bcdDevice*/
    0x02,
    0x01,   /* iManufacturer : index of string Manufacturer  */
    0x02,   /* iProduct      : index of string descriptor of product*/
    0x03,   /* iSerialNumber : index of string serial number*/
    0x01    /*bNumConfigurations */
};

ONE_DESCRIPTOR usbDeviceDescriptorDFU = {
    u8_usbDeviceDescriptorDFU,
    0x12
};

u8 u8_usbFunctionalDescriptor[9] = {
    /******************** DFU Functional Descriptor********************/
    0x09,   /*blength = 7 Bytes*/
    0x21,   /* DFU Functional Descriptor*/
    0x03,   /*bmAttributes, bitCanDnload | bitCanUpload */
    0xFF,   /*DetachTimeOut= 255 ms*/
    0x00,
    (dummyTransferSize & 0x00FF),
    (dummyTransferSize & 0xFF00) >> 8, /* TransferSize = 1024 Byte*/
    0x10,                          /* bcdDFUVersion = 1.1 */
    0x01
};

ONE_DESCRIPTOR usbFunctionalDescriptor = {
    u8_usbFunctionalDescriptor,
    0x09
};

#define u8_usbConfigDescriptorDFU_LENGTH 45
u8 u8_usbConfigDescriptorDFU[u8_usbConfigDescriptorDFU_LENGTH] = {
    0x09,   /* bLength: Configuation Descriptor size */
    0x02,   /* bDescriptorType: Configuration */
    u8_usbConfigDescriptorDFU_LENGTH,   /* wTotalLength: Bytes returned */
    0x00,
    0x01,   /* bNumInterfaces: 1 interface */
    0x01,   /* bConfigurationValue: */
    0x00,   /* iConfiguration: */
    0x80,   /* bmAttributes: */
    0x32,   /* MaxPower 100 mA */
    /* 09 */

    /************ Descriptor of DFU interface 0 Alternate setting 0 *********/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x00,   /* bNumEndpoints*/
    0xFE,   /* bInterfaceClass: DFU */
    0x01,   /* bInterfaceSubClass */

    0x02,   /* nInterfaceProtocol, switched to 0x02 while in dfu_mode */

    0x04,   /* iInterface: */

    /************ Descriptor of DFU interface 0 Alternate setting 1 *********/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x01,   /* bAlternateSetting: Alternate setting */
    0x00,   /* bNumEndpoints*/
    0xFE,   /* bInterfaceClass: DFU */
    0x01,   /* bInterfaceSubClass */

    0x02,   /* nInterfaceProtocol, switched to 0x02 while in dfu_mode */

    0x05,   /* iInterface: */

    /************ Descriptor of DFU interface 0 Alternate setting 2 *********/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x02,   /* bAlternateSetting: Alternate setting */
    0x00,   /* bNumEndpoints*/
    0xFE,   /* bInterfaceClass: DFU */
    0x01,   /* bInterfaceSubClass */

    0x02,   /* nInterfaceProtocol, switched to 0x02 while in dfu_mode */

    0x06,   /* iInterface: */


    /******************** DFU Functional Descriptor********************/
    0x09,   /*blength = 7 Bytes*/
    0x21,   /* DFU Functional Descriptor*/
    0x03,   /*bmAttributes, bitCanDnload | bitCanUpload */
    0xFF,   /*DetachTimeOut= 255 ms*/
    0x00,
    (dummyTransferSize & 0x00FF),
    (dummyTransferSize & 0xFF00) >> 8, /* TransferSize = 1024 Byte*/
    0x10,                          /* bcdDFUVersion = 1.1 */
    0x01
    /***********************************************************/
    /*36*/
};

ONE_DESCRIPTOR usbConfigDescriptorDFU = {
    u8_usbConfigDescriptorDFU,
    u8_usbConfigDescriptorDFU_LENGTH
};

#define USB_STR_LANG_ID_LEN 0x04
u8 u8_usbStringLangId[USB_STR_LANG_ID_LEN] = {
    USB_STR_LANG_ID_LEN,
    0x03,
    0x09,
    0x04    /* LangID = 0x0409: U.S. English */
};

/*
----------- ADDED BY xvever12 -----------
    Manufacturer changed to: "Jiri Veverka"
-----------------------------------------
*/
#define USB_VENDOR_STR_LEN 0x1C
u8 u8_usbStringVendor[USB_VENDOR_STR_LEN] = {
    USB_VENDOR_STR_LEN,
    0x03,
    'J', 0, 'i', 0, 'r', 0, 'i', 0, ' ', 0, 'V', 0, 'e', 0, 'v', 0, 'e', 0, 'r', 0, 'k', 0, 'a'
};
/*
----------- END OF SECTION ------------
*/

/*
----------- CHANGED BY xvever12 -----------
    Changed the product to "JaQT bootloader"
-----------------------------------------
*/
#define USB_PRODUCT_STR_LEN 0x20
u8 u8_usbStringProduct[USB_PRODUCT_STR_LEN] = {
    USB_PRODUCT_STR_LEN,
    0x03,
    'J', 0, 'a', 0, 'Q', 0, 'T', 0, ' ', 0, 'b', 0, 'o', 0, 'o', 0, 't', 0, 'l', 0, 'o', 0, 'a', 0, 'd', 0, 'e', 0, 'r'
};
/*
----------- END OF SECTION ------------
*/

#define USB_SERIAL_STR_LEN 0x40
u8 u8_usbStringSerial[USB_SERIAL_STR_LEN] = {
    USB_SERIAL_STR_LEN,
    0x03,
    '0'
};


/*
----------- ADDED BY xvever12 -----------
    Initializes board serial number on start

    - The serial number is assigned as a value
      in usbStringSerial variable, which represents
      SerialNumber string in USB interface.
-----------------------------------------
*/
#define EEPROM_BOARD_NO ((u32)0x0801F7DC)
void init_board_no() {
    u8 *pmem_data = (u8 *)(EEPROM_BOARD_NO);
    u8 *pchecksum_data = (u8 *)(EEPROM_BOARD_NO) + 1;

    if (*pmem_data + *pchecksum_data == 255) {
        int mem_data = *pmem_data;

        u8 units = mem_data % 10;
        mem_data = mem_data / 10;

        u8 decimals = mem_data % 10;
        mem_data = mem_data / 10;

        u8 hundreds = mem_data % 10;

        if (hundreds) {
            u8_usbStringSerial[2] = hundreds+'0';
            u8_usbStringSerial[3] = 0;

            u8_usbStringSerial[4] = decimals+'0';
            u8_usbStringSerial[5] = 0;

            u8_usbStringSerial[6] = units+'0';
            u8_usbStringSerial[7] = 0;
        } else if (decimals) {
            u8_usbStringSerial[2] = decimals+'0';
            u8_usbStringSerial[3] = 0;

            u8_usbStringSerial[4] = units+'0';
            u8_usbStringSerial[5] = 0;
        } else {
            u8_usbStringSerial[2] = units+'0';
            u8_usbStringSerial[3] = 0;
        }
    }
}
/*
----------- END OF SECTION ------------
*/


    u8 u8_usbStringAlt0[ALT0_STR_LEN] = {
    ALT0_STR_LEN,
    0x03,
    ALT0_MSG_STR
    };


    u8 u8_usbStringAlt1[ALT1_STR_LEN] = {
    ALT1_STR_LEN,
    0x03,
    ALT1_MSG_STR
    };


    u8 u8_usbStringAlt2[ALT2_STR_LEN] = {
    ALT2_STR_LEN,
    0x03,
    ALT2_MSG_STR
    };

u8 u8_usbStringInterface = NULL;

ONE_DESCRIPTOR usbStringDescriptor[STR_DESC_LEN] = {
    { (u8 *)u8_usbStringLangId,  USB_STR_LANG_ID_LEN },
    { (u8 *)u8_usbStringVendor,  USB_VENDOR_STR_LEN },
    { (u8 *)u8_usbStringProduct, USB_PRODUCT_STR_LEN },
    { (u8 *)u8_usbStringSerial,  USB_SERIAL_STR_LEN },
    { (u8 *)u8_usbStringAlt0,    ALT0_STR_LEN },
    { (u8 *)u8_usbStringAlt1,    ALT1_STR_LEN },
    { (u8 *)u8_usbStringAlt2,    ALT2_STR_LEN }
};


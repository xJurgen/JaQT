#include "honeywell.h"

#include <stdio.h>
#include <string.h>

#include "../../../control_shell/usbshell.h"
#include "../../../usb/usartconf.h"
#include "../../../usb/cdcacm.h"

#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>

int rs485_test;
int rs485_tim_wait;
int rs485_sent;
int time_failed_rs485_test;
int data_failed_rs485_test;

static void set_test_rs485()
{
	setUSART(19200, USART3);
}

void begin_test_rs485()
{
	set_test_rs485();
	test_rs485();
}

void delay_test_rs485()
{
	uint32_t t = 0;
	rs485_tim_wait = 1;
	timer_enable_counter(TIM1);
	timer_enable_irq(TIM1, TIM_DIER_UIE);
	while ((timer_get_counter(TIM1) + t) < 5000);
	timer_disable_counter(TIM1);
	timer_disable_irq(TIM1, TIM_DIER_UIE);
	timer_set_period(TIM1, 4500U);
	timer_set_counter(TIM1, 0);
	rs485_tim_wait = 0;
}

void test_rs485()
{
	timer_disable_counter(TIM1);
	timer_set_period(TIM1, 10000U);
	timer_set_counter(TIM1, 0);

	delay_test_rs485();

	//send_usbd_packet(device, VIRTUAL_OUT, "Test\r\n", sizeof("Test\r\n"), 0);
	if (!rs485_test) //If not running, start testing by incrementing the counter to 1
		rs485_test++;
	usart_send_blocking(USART3, 0x05);
	usart_send_blocking(USART3, 0x21);
	usart_send_blocking(USART3, 0x00);
	rs485_sent = 1;

	timer_enable_counter(TIM1);
	timer_enable_irq(TIM1, TIM_DIER_UIE);
}

void stop_test_rs485()
{
	//First set test parameters
	setUSART(115200, USART3);
	timer_disable_counter(TIM1);
	timer_set_period(TIM1, 2010U);
	timer_set_counter(TIM1, 0);

	char num_of_tests[25];
	rs485_test--; //Decrement counter by 1 (when starting test, counter is incremented from 0 to 1)
	sprintf(num_of_tests, "Tests: %d\r\n", rs485_test);
	send_usbd_packet(usbdev, VIRTUAL_OUT, num_of_tests, strlen(num_of_tests), 0);
	rs485_test = 0;

	char num_of_time_failed_tests[25];
	sprintf(num_of_time_failed_tests, "Time fail: %d\r\n", time_failed_rs485_test);
	send_usbd_packet(usbdev, VIRTUAL_OUT, num_of_time_failed_tests, strlen(num_of_time_failed_tests), 0);
	time_failed_rs485_test = 0;

	char num_of_data_failed_tests[30];
	sprintf(num_of_data_failed_tests, "Data mismatch fail: %d\r\n", data_failed_rs485_test);
	send_usbd_packet(usbdev, VIRTUAL_OUT, num_of_data_failed_tests, strlen(num_of_data_failed_tests), 0);
	data_failed_rs485_test = 0;
}

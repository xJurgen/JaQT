/*
*
*	Author: Jiří Veverka (xvever12):
*	Module implementing wiegand communicaton
*
*/


#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

#include "../usb/cdcacm.h"
#include "wiegand.h"
#include "../control_shell/wiegandcommands.h"
#include "../general.h"
#include "../control_shell/usbshell.h"
#include "../control_shell/usbstrings.h"
#include "../periph/exti/exticonf.h"
#include <string.h>

#define DATA_TIME 50
#define PAUSE_TIME 1950
#define TIME_OFFSET 5

uint16_t exti8_direction = FALLING;
uint16_t exti9_direction = FALLING;

char message[6];
char pause[6];

int errDataTime;
int errData;

int errPauseTime;
int errPause;

char wiegandNum[WIEGAND_MAX_SIZE];
uint32_t time0[WIEGAND_MAX_SIZE];
uint32_t time1[WIEGAND_MAX_SIZE];
uint32_t pause0[WIEGAND_MAX_SIZE];
uint32_t pause1[WIEGAND_MAX_SIZE];

int counter = 0;
int write = 0;

int timInUse = 0;

//Indicates if we wait for an end of data signal and which one is it
int wait = 0;
int sig8 = 0;
int sig9 = 0;

//Indicates wiegand error
typedef enum {
	NO_ERR,
	ERR_LINE_0,
	ERR_LINE_1,
	ERR_WRONG_ORDER,
	ERR_TIMEOUT,
	ERR_WRONG_TIME_INTERVAL
} wiegandError;

wiegandError wgerror = NO_ERR;

/*	Wiegand on/off variable and supporting functions */
uint8_t wiegandFlag = 0;
void startWiegand()
{
	wiegandFlag = 1;
}

void stopWiegand()
{
	wiegandFlag = 0;
}

uint8_t getWiegandStatus()
{
	return wiegandFlag;
}

//Wiegand packet size. Default value is 26
uint8_t wiegandSize = 26;
void setWiegandSize(uint8_t size)
{
	wiegandSize = size;
}

uint8_t getWiegandSize()
{
	return wiegandSize;
}

/*	Continuous wiegand on/off variable and supporting functions */
uint8_t continuousWiegand = 0;
void startContinuousWiegand()
{
	continuousWiegand = 1;
}

void stopContinuousWiegand()
{
	continuousWiegand = 0;
}

uint8_t getContinuousWiegandStatus()
{
	return continuousWiegand;
}

void wiegandReset()
{
	for (int i = 0; i < WIEGAND_MAX_SIZE; i++)
	{
		if (i < getWiegandSize())
			wiegandNum[i] = '-';
		else
			wiegandNum[i] = ' ';

		time0[i] = 0;
		time1[i] = 0;
		pause0[i] = 0;
		pause1[i] = 0;
	}
	counter = 0;
}

void errReset(wiegandError errType, uint32_t periph)
{
	wgerror = errType;
	getWiegandStatus();
	wiegandReset();

	exti_reset_request(periph);
	timer_enable_irq(TIM1, TIM_DIER_UIE);
}

void d0_isr()
{
	exti_reset_request(EXTI8);

	if (exti8_direction == RISING) { //Is up, write number as ok. Set trigger for next drop (next num reading start)
		time1[counter] = timer_get_counter(TIM1);

		wait = 0;
		sig8 = 0;
		exti8_direction = FALLING;
		if (counter == getWiegandSize())
			counter = 0;
		wiegandNum[counter++] = '0';

		timer_set_counter(TIM1, 0);
		pause0[counter-1] = timer_get_counter(TIM1);
		timInUse = 1;

		exti_set_trigger(EXTI8, EXTI_TRIGGER_FALLING);
	} else {
		pause1[counter-1] = timer_get_counter(TIM1);
		timInUse = 0;

		timer_set_counter(TIM1, 0);
		time0[counter] = timer_get_counter(TIM1);
		wait = 1;
		sig8 = 1;
		exti8_direction = RISING;
		exti_set_trigger(EXTI8, EXTI_TRIGGER_RISING);
	}
}

void d1_isr(){
	exti_reset_request(EXTI9);
	
	if (exti9_direction == RISING) {
		time1[counter] = timer_get_counter(TIM1);

		wait = 0;
		sig9 = 0;
		exti9_direction = FALLING;
		if (counter == getWiegandSize())
			counter = 0;
		wiegandNum[counter++] = '1';

		timer_set_counter(TIM1, 0);
		pause0[counter-1] = timer_get_counter(TIM1);
		timInUse = 1;

		exti_set_trigger(EXTI9, EXTI_TRIGGER_FALLING);
	} else {
		pause1[counter-1] = timer_get_counter(TIM1);
		timInUse = 0;

		timer_set_counter(TIM1, 0);
		time0[counter] = timer_get_counter(TIM1);
		wait = 1;
		sig9 = 1;
		exti9_direction = RISING;
		exti_set_trigger(EXTI9, EXTI_TRIGGER_RISING);
	}
}

void exti9_5_isr(void)
{	
	if (getWiegandStatus()) {

		if (((wait && !exti8_direction) && sig8) || ((wait && !exti9_direction) && sig9)) {
			//error, packet has arrived in wrong interval
			errReset(ERR_WRONG_ORDER, EXTI8 | EXTI9);
			return;
		}

		if (exti_get_flag_status(EXTI8) != 0) {
			if((sig8 && wait) || !wait){
				d0_isr();
			} else {
				if (getWiegandStatus()) {
					errReset(ERR_LINE_0, EXTI8 | EXTI9);
                    return;
				}
			}
		}
		else if (exti_get_flag_status(EXTI9) != 0) {
			if ((sig9 && wait) || !wait) {
				d1_isr();
			} else {
				if(getWiegandStatus()) {
					errReset(ERR_LINE_1, EXTI8 | EXTI9);
                    return;
				}
			}
		}

		if ((counter == getWiegandSize()) && getWiegandStatus()) {
			stopWiegand();
			write = 1;
		}
		timer_enable_irq(TIM1, TIM_DIER_UIE);
	} else {
		exti_reset_request(EXTI8 | EXTI9);
	}
}

//int i_rs485;
void tim1_up_isr(void)
{
	timer_clear_flag(TIM1, TIM_SR_UIF);

	if (rs485_tim_wait) return;

	//If rs485 test is active, we end the function evaluation here
	if (rs485_test) {
		if (rs485_sent) {
			rs485_sent = 0;
			rs485_test++;
			timer_disable_irq(TIM1, TIM_DIER_UIE);
			//send_usbd_packet(usbdev, VIRTUAL_OUT, "Failed!\r\n", sizeof("Failed!\r\n"), 0);
			time_failed_rs485_test++;
			test_rs485();
		} else {
			send_usbd_packet(usbdev, VIRTUAL_OUT, "Undefined err\r\n", sizeof("Undefined err\r\n"), 0);
		}
		return;
	}

	if (timInUse && !write) {
		timer_disable_irq(TIM1, TIM_DIER_UIE);

		errPause = counter-1;
		wgerror = ERR_TIMEOUT;
		stopWiegand();
		timInUse = 0;
	}

	if(write) {
		timer_disable_irq(TIM1, TIM_DIER_UIE);

		timInUse = 0;

		for (int i = 0; i < getWiegandSize(); i++)
		{
			errData = i;
			errDataTime = time1[i] - time0[i];
			if (errDataTime > (DATA_TIME + TIME_OFFSET) || errDataTime < (DATA_TIME - TIME_OFFSET)) {
				wgerror = ERR_WRONG_TIME_INTERVAL;
				stopWiegand();
				write = 0;
				break;
			}
		}
		for(int i = 0; i < getWiegandSize()-1; i++)
		{
			errPause = i;
			if (pause1[i] > pause0[i])
				errPauseTime = pause1[i] - pause0[i];
			else
				errPauseTime = ((PAUSE_TIME + DATA_TIME) + TIME_OFFSET) * 2 + pause0[i] - pause1[i];

			if (errPauseTime > (PAUSE_TIME + TIME_OFFSET*2) || errPauseTime < (PAUSE_TIME - TIME_OFFSET*2)) {
				wgerror = ERR_TIMEOUT;
				stopWiegand();
				write = 0;
				break;
			}
		}
	}
	if(write) {
		write = 0;
		send_usbd_packet(usbdev, VIRTUAL_OUT, bufMessageLining, sizeof(bufMessageLining), 0);
		send_usbd_packet(usbdev, VIRTUAL_OUT, labelOK, sizeof(labelOK), 0);
		send_usbd_packet(usbdev, VIRTUAL_OUT, wiegandDone, sizeof(wiegandDone), 0);
		send_usbd_packet(usbdev, VIRTUAL_OUT, dataText, sizeof(dataText), 0);
		send_usbd_packet(usbdev, VIRTUAL_OUT, wiegandNum, getWiegandSize(), 0);
		
		send_usbd_packet(usbdev, VIRTUAL_OUT, dataTimes, sizeof(dataTimes), 0);
		for (int i = 0; i < getWiegandSize(); i++)
		{
			sprintf(message, "%lu ",time1[i] - time0[i]);
			send_usbd_packet(usbdev, VIRTUAL_OUT, message, strlen(message), 0);
		}
		send_usbd_packet(usbdev, VIRTUAL_OUT, pauseTimes, sizeof(pauseTimes), 0);
		for (int i = 0; i < getWiegandSize()-1; i++)
		{
			if (pause1[i] > pause0[i])
				sprintf(pause, "%lu ", pause1[i] - pause0[i]);
			else
				sprintf(pause, "%lu ", ((PAUSE_TIME + DATA_TIME) + TIME_OFFSET * 2) + pause0[i] - pause1[i]);
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause, strlen(pause), 0);
		}

#if WIEGAND_DEBUG
		send_usbd_packet(usbdev, VIRTUAL_OUT, time0Times, sizeof(time0Times), 0);
		for (int i = 0; i < getWiegandSize(); i++)
		{
			sprintf(pause, "%lu ", time0[i]);
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
		}
		send_usbd_packet(usbdev, VIRTUAL_OUT, time1Times, sizeof(time1Times), 0);
		for (int i = 0; i < getWiegandSize(); i++)
		{
			sprintf(pause, "%lu ", time1[i]);
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
		}
		send_usbd_packet(usbdev, VIRTUAL_OUT, pause0Times, sizeof(pause1Times), 0);
		for (int i = 0; i < getWiegandSize()-1; i++)
		{
			sprintf(pause, "%lu ", pause0[i]);
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
		}
		send_usbd_packet(usbdev, VIRTUAL_OUT, pause1Times, sizeof(pause1Times), 0);
		for (int i = 0; i < getWiegandSize()-1; i++)
		{
			sprintf(pause, "%lu ", pause1[i]);
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
		}	
#endif
		
		send_usbd_packet(usbdev, VIRTUAL_OUT, "\r\n", sizeof("\r\n"), 0);
		send_usbd_packet(usbdev, VIRTUAL_OUT, bufMessageLining, sizeof(bufMessageLining), 0);

		wiegandReset();

		counter = 0;
		if (getContinuousWiegandStatus() && wgerror == NO_ERR) {
			startWiegand();
		} else {
			stopWiegand();
			stopContinuousWiegand();
			setWiegandSize(26);
		}
		return;
	}
	if (wgerror && platform_ready) {
		if (wgerror == ERR_LINE_0) {
			send_usbd_packet(usbdev, VIRTUAL_OUT, labelERROR, sizeof(labelERROR), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, line0Error, sizeof(line0Error), 0);
		} else if (wgerror == ERR_LINE_1) {
			send_usbd_packet(usbdev, VIRTUAL_OUT, labelERROR, sizeof(labelERROR), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, line1Error, sizeof(line1Error), 0);
		} else if (wgerror == ERR_WRONG_ORDER) {
			send_usbd_packet(usbdev, VIRTUAL_OUT, labelERROR, sizeof(labelERROR), 0);
			char errpacket[4];
			sprintf(errpacket, " %d", counter);
			send_usbd_packet(usbdev, VIRTUAL_OUT, errpacket, sizeof(errpacket), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, wiegandPacketErr, sizeof(wiegandPacketErr), 0);
		} else if (wgerror == ERR_TIMEOUT) { //PAUSE
			char errpacket[4];
			errPause++;
			sprintf(errpacket, "%d", errPause);
			send_usbd_packet(usbdev, VIRTUAL_OUT, labelERROR, sizeof(labelERROR), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, packetNumber, sizeof(packetNumber), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, errpacket, strlen(errpacket), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, timeoutWaitingErr, sizeof(timeoutWaitingErr), 0);

			if (counter != getWiegandSize()) {
				send_usbd_packet(usbdev, VIRTUAL_OUT, missingPackets, sizeof(missingPackets), 0);
			} else {
				send_usbd_packet(usbdev, VIRTUAL_OUT, timeoutText, sizeof(timeoutText), 0);
			}


			send_usbd_packet(usbdev, VIRTUAL_OUT, dataText, sizeof(dataText), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, wiegandNum, getWiegandSize(), 0);
			
			send_usbd_packet(usbdev, VIRTUAL_OUT, dataTimes, sizeof(dataTimes), 0);
			for (int i = 0; i < counter; i++)
			{
				sprintf(message, "%lu ",time1[i] - time0[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, message, strlen(message), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, pauseTimes, sizeof(pauseTimes), 0);
			for (int i = 0; i < counter-1; i++)
			{
				if (i == (errPause - 1)) send_usbd_packet(usbdev, VIRTUAL_OUT, "->", sizeof("->"), 0);
				if (pause1[i] > pause0[i])
					sprintf(pause, "%lu ", pause1[i] - pause0[i]);
				else
					sprintf(pause, "%lu ", ((PAUSE_TIME + DATA_TIME) + TIME_OFFSET * 2) + pause0[i] - pause1[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, strlen(pause), 0);
			}
			if (errPause > counter-1) send_usbd_packet(usbdev, VIRTUAL_OUT, "->X", sizeof("->X"), 0);

#if WIEGAND_DEBUG
			send_usbd_packet(usbdev, VIRTUAL_OUT, time0Times, sizeof(time0Times), 0);
			for (int i = 0; i < counter; i++)
			{
				sprintf(pause, "%lu ", time0[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, time1Times, sizeof(time1Times), 0);
			for (int i = 0; i < counter; i++)
			{
				sprintf(pause, "%lu ", time1[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause0Times, sizeof(pause1Times), 0);
			for (int i = 0; i < counter-1; i++)
			{
				sprintf(pause, "%lu ", pause0[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause1Times, sizeof(pause1Times), 0);
			for (int i = 0; i < counter-1; i++)
			{
				sprintf(pause, "%lu ", pause1[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
#endif

			send_usbd_packet(usbdev, VIRTUAL_OUT, "\r\n", sizeof("\r\n"), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, bufMessageLining, sizeof(bufMessageLining), 0);
		} else if (wgerror == ERR_WRONG_TIME_INTERVAL) { //DATA
			char errpacket[4];
			errData++; //Error data index +1
			sprintf(errpacket, "%d", errData);
			send_usbd_packet(usbdev, VIRTUAL_OUT, labelERROR, sizeof(labelERROR), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, packetNumber, sizeof(packetNumber), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, errpacket, strlen(errpacket), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, wrongInterval, sizeof(wrongInterval), 0);
			char errtime[4];
			sprintf(errtime, "%d", errDataTime);
			send_usbd_packet(usbdev, VIRTUAL_OUT, errtime, strlen(errtime), 0);

			send_usbd_packet(usbdev, VIRTUAL_OUT, dataText, sizeof(dataText), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, wiegandNum, getWiegandSize(), 0);
			
			send_usbd_packet(usbdev, VIRTUAL_OUT, dataTimes, sizeof(dataTimes), 0);
			for (int i = 0; i < counter; i++)
			{
				if (i == (errData-1)) send_usbd_packet(usbdev, VIRTUAL_OUT, "->", sizeof("->"), 0);
				sprintf(message, "%lu ",time1[i] - time0[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, message, strlen(message), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, pauseTimes, sizeof(pauseTimes), 0);
			for (int i = 0; i < counter-1; i++)
			{
				if (pause1[i] > pause0[i])
					sprintf(pause, "%lu ", pause1[i] - pause0[i]);
				else
					sprintf(pause, "%lu ", ((PAUSE_TIME + DATA_TIME) + TIME_OFFSET * 2) + pause0[i] - pause1[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, strlen(pause), 0);
			}

#if WIEGAND_DEBUG
			send_usbd_packet(usbdev, VIRTUAL_OUT, time0Times, sizeof(time0Times), 0);
			for (int i = 0; i < counter; i++)
			{
				sprintf(pause, "%lu ", time0[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, time1Times, sizeof(time1Times), 0);
			for (int i = 0; i < counter; i++)
			{
				sprintf(pause, "%lu ", time1[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause0Times, sizeof(pause1Times), 0);
			for (int i = 0; i < counter-1; i++)
			{
				sprintf(pause, "%lu ", pause0[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
			send_usbd_packet(usbdev, VIRTUAL_OUT, pause1Times, sizeof(pause1Times), 0);
			for (int i = 0; i < counter-1; i++)
			{
				sprintf(pause, "%lu ", pause1[i]);
				send_usbd_packet(usbdev, VIRTUAL_OUT, pause, sizeof(pause), 0);
			}
#endif

			send_usbd_packet(usbdev, VIRTUAL_OUT, "\r\n", sizeof("\r\n"), 0);
			send_usbd_packet(usbdev, VIRTUAL_OUT, bufMessageLining, sizeof(bufMessageLining), 0);
		}

		counter = 0;
		errReset(0, EXTI8 | EXTI9);
		if (wgerror != 1 || wgerror != 2)
			exti_set_trigger(EXTI8 | EXTI9, EXTI_TRIGGER_FALLING);
		wgerror = NO_ERR;
		stopWiegand();
		stopContinuousWiegand();
		setWiegandSize(26);
		return;
	}
}

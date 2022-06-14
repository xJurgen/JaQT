#ifndef __RS485_HONEYWELL_H
#define __RS485_HONEYWELL_H

extern int rs485_test;
extern int rs485_tim_wait;
extern int rs485_sent;
extern int time_failed_rs485_test;
extern int data_failed_rs485_test;

void begin_test_rs485();
void delay_test_rs485();
void test_rs485();
void stop_test_rs485();

#endif

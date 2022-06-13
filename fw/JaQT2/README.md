JaQT2: 3-USB-to-serial + command shell
======================================

Triple USB-to-serial adapter firmware for flashing onto an STM32F103C8T6 "blue pill" minimum development board.

Run `make` to build or `make flash BOARD_NO=number [PORT=shellport]` to flash (PORT is not necessary when flashing for the first time)bin` file to a blue pill over the
Afterh flash three virtual (ACM CDC, "/dev/usbmodem") serial ports should appear.
These correspond to the three USART ports available on the board, in order:

| TX pin | RX pin | Port | Serial |
| ------ | ------ | ---- | ------ |
| PB10   | PB11   | ACM0 | RS232  |
| PA2    | PA3    | ACM1 | RS485  |
| PA9    | PA10   | ACM2 | Debug  |
| -      | -      | ACM3 | Shell  |

This code is based on the [pill_serial](https://github.com/satoshinm/pill_serial).

---

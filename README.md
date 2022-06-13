<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary><h2 style="display: inline-block">Table of Contents</h2></summary>
  <ol>
    <li>
      <a href="#JaQT-(Just-a-Quikt-Test-)">JaQT (Just a Quick Test)</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#preparation-and-bootloader-upload">Preparation and bootloader upload</a></li>
        <li><a href="#compilation-and-flash">Compilation and flash</a></li>
      </ul>
    </li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#acknowledgements">Acknowledgements</a></li>
  </ol>
</details>


## JaQT (Just a Quick Test)

Just a Quick Test is an fw app designed for HW testing running on top of a bootloader. 
It can be used to test communications (usb, RS232, RS485, wiegand), read/write up to 14 GPIOs or control devices over I2C.

In fw folder are source files for JaQT (app) and bootloader.
Folder sw contains Python scripts for use on top of flashed and running JaQT board.

Firmware implementation is based on user expansion board (see acknowledgments). Project can be easily modified to suit your needs.

If you find a description of "JaQT2" somewhere in this project, don't panic - this is just a newer, compatible version of the project.

## Getting Started

### Prerequisites

List of a software you'll need:

  * dfu-util
  * stlink-tools
  * gcc-arm-none-eabi

### Preparation and bootloader upload
1. Connect ST-LINK to the device as pictured on the image in the documentation
2. Upload bootloader(+app) to the device (address 0x8000000):
```
 st-flash --reset write <chosen_bootloader.bin> 0x8000000
```

### Compilation and flash
Following are some examples of compile and flash

- Compile bootloader, JaQT app and merged (bootloader+app) binaries:
```
 make
```
- Compile and flash JaQT binary for first time (ST-LINK connected):
```
 make flash
```
- Compile and flash JaQT binary as an update (USB DFU interface):

```
 make flash PORT=x [BOARD_NO=y]
```
_x stands for DFU port interface (eg. /dev/ttyACM3), y is flashed board number (eg. 3)_

_Another make examples can be found in short-info.md_

_Note: PORT=x means the shell port, eg. PORT=/dev/ttyACM3_

### Setting board number

After initial flash, board number is not set and requires to be set manually. To do so,
open JaQT2 control interface (usually using screen on port /dev/ttyACM3 with baudrate 115200) and run command:

```
board_no set [number]
```
After successful change the board should restart itself automatically. To check if board number has been successfully changed,
check board number checksum (should be 255 - [number]) using command:

```
board_no checksum
```


### Acknowledgements

* JaQT2 application based on: [pill_serial](https://github.com/satoshinm/pill_serial)
* Bootloader used (with some changes): [STM32duino-bootloader](https://github.com/rogerclarkmelbourne/STM32duino-bootloader)
* Library used for JaQT2 app: [libopencm3](https://github.com/libopencm3/libopencm3)
* Expansion board from Radim Pavlik: [jaqt-hw](https://github.com/RadimPavlik/JaQT2)

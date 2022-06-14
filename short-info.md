To compile everything and copy resulting binaries to build dir run: make

To compile bootloader run: make bootloader
To copy compiled bootloader run: make copy_bootloader

To compile JaQT2 app run: make app
To copy compiled JaQT2 app run: make copy_app

To merge app and bootloader into one binary, run: make merge

To clean workspace run: make clean

To flash device with bootloader run st-flash --reset write <choosen_bootloader.bin> 0x8000000

To update app on already running device use make flash PORT=/dev/ttyACM<number of shell port, typically a (multiple of 4) - 1>

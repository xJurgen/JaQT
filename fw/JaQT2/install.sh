#!/bin/bash

read -p "Install gcc-arm-none-eabi, python and dfu-util (requires superuser privileges)? [Y/N]" input

if [[ $input == 'Y' ]] || [[ $input == 'y' ]]; then
	sudo apt install gcc-arm-none-eabi python dfu-util
fi

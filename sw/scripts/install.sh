#!/bin/bash

read -p "Install pip? [Y/N]" input

if [[ $input == 'Y' ]] || [[ $input == 'y' ]]; then
	sudo apt-get install pip
fi

read -p "Install pyserial (requires pip)? [Y/N]" pyinput

if [[ $pyinput == 'Y' ]] || [[ $pyinput == 'y' ]]; then
	pip install pyserial
fi

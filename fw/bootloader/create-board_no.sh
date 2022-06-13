#!/usr/bin/env bash

#
#   Author: Jiří Veverka
#   Script for generating static serial board numbers (obsolete, keeping for backward compatibility)
#

size=${#1}
num=$1
BOARD_NO=""
for ((i = 0; i < $size; i++))
do
    BOARD_NO+=" '"
    BOARD_NO+=${num:$i:1};
    BOARD_NO+="', 0"
    if (("$i+1" < "$size")); then
        BOARD_NO+=","
    fi
done
echo $BOARD_NO

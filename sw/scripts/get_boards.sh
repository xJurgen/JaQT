#!/bin/bash

JAQT_INTERFACES=($(ls /dev/ | grep ttyACM))

echo "Interfaces: "
echo 

MAX_INTERFACE_NUM=0

for i in ${!JAQT_INTERFACES[@]}; do
	INTERFACE_NUM=$(grep -Eo '[0-9]+' <<<"${JAQT_INTERFACES[$i]}")
	if [ $INTERFACE_NUM -ge $MAX_INTERFACE_NUM ]; then
		MAX_INTERFACE_NUM=$INTERFACE_NUM
	fi
done

MAX_INTERFACE_NUM=$((MAX_INTERFACE_NUM+1))

for ((index = 3; index < $MAX_INTERFACE_NUM; index=index+4)) do
	INTERFACE="/dev/ttyACM"$index
	PYTHON_OUTPUT=$(./get_boardinfo.py $INTERFACE | tr -d '\0')
	
	REPLACE_ACM=${PYTHON_OUTPUT//ACM0/ACM$((index-3))}
	REPLACE_ACM=${REPLACE_ACM//ACM1/ACM$((index-2))}
	REPLACE_ACM=${REPLACE_ACM//ACM2/ACM$((index-1))}
	REPLACE_ACM=${REPLACE_ACM//ACM3/ACM$index}
	
	echo $INTERFACE
	echo "$REPLACE_ACM"
done

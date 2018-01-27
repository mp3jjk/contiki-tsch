#!/bin/bash
SKIP=0
NODE=100
PORTS=(2)

for opt in $@
do
	case "$opt" in
		"k" ) SKIP=1;;
	  *) NODE=$opt;;
	esac
done


for PORT in "${PORTS[@]}"
do
	if [ $SKIP -eq 0 ]
	then
		sudo make clean TARGET=zoul BOARD=firefly
		sudo make test TARGET=zoul BOARD=firefly
	fi
	NODEID="0x$NODE"
	sudo make test.upload NODEID=$NODEID TARGET=zoul PORT=/dev/ttyUSB$PORT BOARD=firefly

	echo
	echo $NODEID
	echo

	let "NODE=$NODE +1"
	SKIP=1
done 

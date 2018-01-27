#!/bin/bash
PORT=1

for opt in $@
do
	case "$opt" in
		* ) PORT=$opt;;
	esac
done

sudo make login TARGET=zoul PORT=/dev/ttyUSB$PORT BOARD=firefly



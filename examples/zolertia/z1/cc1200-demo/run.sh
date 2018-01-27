#! /bin/bash

make clean
make cc1200-demo
sudo make cc1200-demo.upload PORT=/dev/ttyUSB0
make login

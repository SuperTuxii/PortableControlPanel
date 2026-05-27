#!/bin/bash
data="\x${1// /\\x}"
stty -F /dev/ttyACM0 115200 raw -echo
echo -en "$data" > /dev/ttyACM0
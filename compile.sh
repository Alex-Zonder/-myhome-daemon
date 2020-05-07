#!/bin/bash

[ ! $(which gcc) ] && echo 'ERROR: gcc not installed!'

OS=`uname`


ignore="1"
if [ $OS == "Darwin" ]; then
  gcc -o /usr/local/bin/myhome main.c && ignore="0"
else
  gcc -pthread -o /bin/myhome main.c && ignore="0"
fi


if [ "$ignore" == "0" ]; then
	echo "COMPILE OK!!!"

	if [ "${1}" != "" ]; then
		sudo killall myhome 2>/dev/null
		sudo myhome "$*"
	fi
else
	echo "COMPILE ERROR!!!"
fi

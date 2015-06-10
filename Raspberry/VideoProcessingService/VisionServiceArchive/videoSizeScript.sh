#!/bin/bash

size=$(du $1 | cut -f1)
if [ $size -gt $2 ]
then
	kill -9 10541
	pkill -9 watch
        exit
fi

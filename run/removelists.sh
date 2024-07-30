#!/bin/bash

COUNTER=0
for file in lists/*.list; do
    if [ $(($COUNTER % 2)) -eq 1 ]; then
	rm $file
    fi
    COUNTER=$(($COUNTER + 1))
done

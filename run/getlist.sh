#!/bin/bash


UPPER=`awk -v n=$1 'BEGIN{print int((n+50)/100) * 100}'`
if [ $UPPER -lt $1 ]; then
    UPPER=$(($UPPER + 100))
fi

LOWER=$((UPPER - 100))

if [ $LOWER -lt 100000 ]; then
    LOWER=000$LOWER
else
    LOWER=00$LOWER
fi

if [ $UPPER -lt 100000 ]; then
    UPPER=000$UPPER
else
    UPPER=00$UPPER
fi

dir="physics"
echo $UPPER
if [ $2 -eq 0 ]; then
    dir="commissioning"
fi

ls /sphenix/lustre01/sphnxpro/$dir/slurp/caloy2test/new_2024p005/run_$LOWER\_$UPPER/*$1* > lists/$1.list

#!/bin/bash

rm longrunlist.txt
for rn in `cat /sphenix/u/bseidlitz/work/test/macros/validation/calo/dbTools/grl_Greg_July9_unofficial.csv | grep GoodRun | awk -F',' '{print $1}'`; do
    if [ $rn -gt 47000 ]; then
	echo $rn >> longrunlist.txt
    fi
done


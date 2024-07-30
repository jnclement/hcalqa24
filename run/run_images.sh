#!/bin/bash

TAG=`echo $1 | awk -F'/' '{print $2}' | awk -F'.' '{print $2}'`
if [ $# -lt 1 ]; then
    echo "Need arguments (in order): filename"
    exit 1
fi

NJOB=`wc -l < $1`
NJOB=$(( ($NJOB + 9) / 10))
BASENAME="condor_${TAG}_${NJOB}_imagemaker"

SUBNAME="${BASENAME}.sub"

echo "executable = quickroot.sh" > $SUBNAME
echo "arguments = ${1} \$(Process)" >> $SUBNAME
echo "output = /sphenix/user/jocl/projects/run2024_hcalQA/run/output/out/output_${BASENAME}_\$(Process).out" >> $SUBNAME
echo "should_transfer_files   = IF_NEEDED" >> $SUBNAME
echo "when_to_transfer_output = ON_EXIT" >> $SUBNAME
echo "error = /sphenix/user/jocl/projects/run2024_hcalQA/run/output/err/error_${BASENAME}_\$(Process).err" >> $SUBNAME
echo "log = /tmp/jocl_${BASENAME}_\$(Process).log" >> $SUBNAME
echo "queue ${NJOB}" >> $SUBNAME

condor_submit $SUBNAME

#!/bin/bash

BASENAME="condor_plotmaker"

SUBNAME="${BASENAME}.sub"

echo "executable = plot.sh" > $SUBNAME
echo "arguments = \$(Process)" >> $SUBNAME
echo "output = /sphenix/user/jocl/projects/run2024_hcalQA/run/output/out/output_${BASENAME}_\$(Process).out" >> $SUBNAME
echo "should_transfer_files   = IF_NEEDED" >> $SUBNAME
echo "when_to_transfer_output = ON_EXIT" >> $SUBNAME
echo "error = /sphenix/user/jocl/projects/run2024_hcalQA/run/output/err/error_${BASENAME}_\$(Process).err" >> $SUBNAME
echo "log = /tmp/jocl_${BASENAME}_\$(Process).log" >> $SUBNAME
echo "queue "`wc -l < lists/sumdatlist.list` >> $SUBNAME

condor_submit $SUBNAME

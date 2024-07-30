if [ $# -lt 1 ]; then
    echo "need sim or dat (0 or 1)"
    exit 1
fi

if [ $1 -eq 1 ]; then
    TYPE=dat
    for rn in `ls  lists/*.imagelist | awk -F'.' '{print $1}' | awk -F'/' '{print $2}'`; do
	BASENAME="condor_${rn}_hadd"

	SUBNAME="${BASENAME}.sub"
	
	echo "executable = haddsingle.sh" > $SUBNAME
	echo "arguments = ${rn} ${TYPE}" >> $SUBNAME
	echo "output = /sphenix/user/jocl/projects/run2024_hcalQA/run/output/out/output_${BASENAME}.out" >> $SUBNAME
	echo "should_transfer_files   = IF_NEEDED" >> $SUBNAME
	echo "when_to_transfer_output = ON_EXIT" >> $SUBNAME
	echo "error = /sphenix/user/jocl/projects/run2024_hcalQA/run/output/err/error_${BASENAME}.err" >> $SUBNAME
	echo "log = /tmp/jocl_${BASENAME}.log" >> $SUBNAME
	echo "queue 1" >> $SUBNAME
	condor_submit $SUBNAME
    done
fi

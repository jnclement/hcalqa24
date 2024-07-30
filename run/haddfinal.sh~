if [ $# -lt 1 ]; then
    echo "Need type (sim or dat 0 or 1)"
fi

TYPE=sim
if [ $1 -eq 1 ]; then
    TYPE=dat
fi

hadd "summed_${TYPE}.root" `ls output/root/*${TYPE}*fullfile*`

#for rn in `ls  /sphenix/tg/tg01/commissioning/CaloCalibWG/bseidlitz/temp24DSTs/*0000* | awk -F'-' '{print $2}'`; do
#    rn=$(expr $rn + 0)
#    bash getlist.sh $rn 1
#done

for rn in $@; do
    bash getlist.sh $rn 1
done

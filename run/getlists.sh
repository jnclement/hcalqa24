#for rn in `ls  /sphenix/tg/tg01/commissioning/CaloCalibWG/bseidlitz/temp24DSTs/*0000* | awk -F'-' '{print $2}'`; do
#    rn=$(expr $rn + 0)
#    bash getlist.sh $rn 1
#done

if [ $# -lt 2 ]; then
    echo "need arguments run number for range of 100 (e.g., enter 45255, get 45200-45300), physics or commissioning (anything or 0, respectively)"
    exit 1
fi

UPPER=`awk -v n=$1 'BEGIN{print int((n+50)/100) * 100}'`
if [ $UPPER -lt $1 ]; then
    UPPER=$(($UPPER + 100))
fi
echo $UPPER
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

if [ $2 -eq 0 ]; then
    dir="commissioning"
fi

for rn in `ls --color=no /sphenix/lustre01/sphnxpro/${dir}/slurp/caloy2test/run_${LOWER}_${UPPER}/*0000* | awk -F'-' '{print $2}'`; do
    rn=$(expr $rn + 0)
    bash getlist.sh $rn $2
done

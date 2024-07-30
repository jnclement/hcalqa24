rm lists/sumdatlist.list
for rn in `ls lists/*.list | awk -F'.' '{print $1}' | awk -F'/' '{print $2}'`; do
    ls output/sumroot/*$rn*dat* > lists/sumdatlist.list
done

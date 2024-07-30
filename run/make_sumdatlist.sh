rm lists/sumdatlist.list
for rn in `ls lists/*.list | awk -F'.' '{print $1}' | awk -F'/' '{print $2}'`; do
    ls output/sumroot/*$rn* > lists/sumdatlist.list
done

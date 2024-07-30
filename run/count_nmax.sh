rm listoflist.txt
for list in lists/*.list; do
    cat $list | head -n 350 >> listoflist.txt
done
wc -l listoflist.txt

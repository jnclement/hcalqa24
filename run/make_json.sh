#!/bin/bash

OUTN=$1"_all.json"
EMCN=$1"_em.txt"
IHCN=$1"_ih.txt"
OHCN=$1"_oh.txt"

if [ $# -lt 1 ]; then
    echo "Need arguments filebase (string)" #, run number (int), event number (int)"
    exit 1
fi

printf "{
    \"EVENT\": {
        \"runid\": 1, 
        \"evtid\": 1, 
        \"time\": 0, 
        \"type\": \"Collision\", 
        \"s_nn\": 0, 
        \"B\": 3.0,
        \"pv\": [0,0,0],
        \"runstats\": [
                \"sPHENIX Experiment at RHIC\",
                \"Collisions: p + p @ √S<sub>NN</sub> = 200 GeV\",
                \"sPHENIX Internal\"
            ]
    },

    \"META\": {
       \"HITS\": {
          \"INNERTRACKER\": {
              \"type\": \"3D\",
              \"options\": {
              \"size\": 5,
              \"color\": 16777215
              } 
          },

    \"CEMC\": {
        \"type\": \"PROJECTIVE\",
        \"options\": {
            \"rmin\": 90,
            \"rmax\": 136.1,
            \"deta\": 0.025,
            \"dphi\": 0.025,
            \"color\": 16766464,
            \"transparent\": 0.6,
            \"scaleminmax\": { \"min\": 0.0310923, \"max\": 6.62854}
              } 
          },

    \"HCALIN\": {
        \"type\": \"PROJECTIVE\",
        \"options\": {
            \"rmin\": 147.27,
            \"rmax\": 175.0,
            \"deta\": 0.025,
            \"dphi\": 0.025,
            \"color\": 4290445312,
            \"transparent\": 0.6,
            \"scaleminmax\": { \"min\": 0.000814855, \"max\": 0.729129}
              }
          },

    \"HCALOUT\": {
        \"type\": \"PROJECTIVE\",
        \"options\": {
            \"rmin\": 183.3,
            \"rmax\": 348.634,
            \"deta\": 0.025,
            \"dphi\": 0.025,
            \"color\": 24773,
            \"transparent\": 0.6,
            \"scaleminmax\": { \"min\": 0.00224448, \"max\": 4.23513}
              }
          },

    \"JETS\": {
        \"type\": \"JET\",
        \"options\": {
            \"rmin\": 0,
            \"rmax\": 78,
            \"emin\": 0,
            \"emax\": 30,
            \"color\": 16777215,
            \"transparent\": 0.5 
        }
      }
     }
   },

\"HITS\": {

\t\"CEMC\":  [" > $OUTN

cat $EMCN | while read line; do
    printf "\t" >> $OUTN
    echo $line >> $OUTN
done

printf "\t],

\"HCALIN\": [" >> $OUTN

cat $IHCN | while read line; do
    echo $line >> $OUTN
done

printf "],

\"HCALOUT\": [" >> $OUTN

cat $OHCN | while read line; do
    echo $line >> $OUTN
done

printf "]
},

\"TRACKS\": {
\"INNTERTRACKER\": []
}
}" >> $OUTN

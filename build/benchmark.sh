#!/bin/bash

# Repeat the tests ($ARG1 ?? 10) times
# REPS=${1:-10}

EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
EXEC_FILE='./stv'

TNONCE=$(date +'%Y-%m-%d-%H-%M-%S')

LOG_FILE1="./logs/${TNONCE}gen.txt"   # generation only
LOG_FILE2="./logs/${TNONCE}verif.txt"   # verification only
LOG_FILE3="./logs/${TNONCE}gverif.txt"  # generation and verification

LOG_FILE4="./logs/${TNONCE}metag.txt"   
LOG_FILE5="./logs/${TNONCE}metav.txt"   

OPTIONS1='--mode 1'
OPTIONS2='--mode 2'
OPTIONS3='--mode 3'

OPTIONS4='--mode 5'
OPTIONS5='--mode 6'

date +"%c" > $LOG_FILE1
date +"%c" > $LOG_FILE2
date +"%c" > $LOG_FILE3
date +"%c" > $LOG_FILE4
date +"%c" > $LOG_FILE5


# iterate over files from catalog
for entry in $(ls -d ../examples/*);do
    familyName=${entry##*/}         # directory name 
    
    if [ "$familyName" = "ssvr" ]; then
        continue
    fi

    for f in $(ls ${entry}); do
        fileName="${f%*.txt}"       # basename of the model
        filePath="${entry}/${f}"
        # echo "$fileName"

        # for (( i=1; i<=5; i++ )); do 
            res1=$($EXEC_TIME $EXEC_FILE $OPTIONS1 --file $filePath 2>&1>/dev/null)
            echo -e "$fileName: [$res1]" >> $LOG_FILE1
            res2=$($EXEC_TIME $EXEC_FILE $OPTIONS2 --file $filePath 2>&1>/dev/null)
            echo -e "$fileName: [$res2]" >> $LOG_FILE2
            res3=$($EXEC_TIME $EXEC_FILE $OPTIONS3 --file $filePath 2>&1>/dev/null)
            echo -e "$fileName: [$res3]" >> $LOG_FILE3
        # done
        
        res4=$($EXEC_FILE $OPTIONS4 --file $filePath)
        echo -e "$fileName: [$res4]" >> $LOG_FILE4

        res5=$($EXEC_FILE $OPTIONS5 --file $filePath)
        echo -e "$fileName: [$res5]" >> $LOG_FILE5
    done
done


# echo $ctr
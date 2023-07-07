#!/bin/bash

# ================================================
# Runs the benchmark tests:
# - measures time and memory use for
#   (1) global model generation 
#   (2) on-the-fly verification
#   (3) global model generation + verification 
# - metadata
#   (4) #st and #trn [ALL]
#   (5) #st and #trn [required for on-the-fly]
# 
# Results are saved in logfiles{1,..,5}.
# 
# An average of repeated measurements could be
# computed via other tool (e.g., nodejs script)
# or by appending an `awk` invocation here.
# 
# Similarly, timeout and memory upper bounds are
# not specified by default (this can be added by
# slightly modifying the code below)
# ================================================

# set current working directory to script location
cd "${0%/*}"

# save prev stack limit
prevStackLim=$(ulimit -s)
ulimit -s 'unlimited'

EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
EXEC_FILE='./stv'

TNONCE=$(date +'%Y-%m-%d-%H-%M-%S')

LOG_FILE1="./logs/${TNONCE}gen.txt"     # generation only
LOG_FILE2="./logs/${TNONCE}verif.txt"   # verification only
LOG_FILE3="./logs/${TNONCE}gverif.txt"  # generation and verification

LOG_FILE4="./logs/${TNONCE}metag.txt"   
LOG_FILE5="./logs/${TNONCE}metav.txt"   

OPTIONS1='--mode 1' # only generate global model
OPTIONS2='--mode 2' # only verify (does not require full unfolding)
OPTIONS3='--mode 3' # generate global model and then verify 

OPTIONS4='--mode 5' # get metadata on global model (complete) and local models
OPTIONS5='--mode 6' # get metadata on global model fragment (used in on-the-fly)

date +"%c" > $LOG_FILE1
date +"%c" > $LOG_FILE2
date +"%c" > $LOG_FILE3
date +"%c" > $LOG_FILE4
date +"%c" > $LOG_FILE5

# iterate over examples
for entry in $(ls -d ../examples/*);do
    familyName=${entry##*/}         # current directory name 
    
    if [ "$familyName" = "ssvr" ]; then
        continue
    fi
    if [ "$familyName" = "svote" ]; then
        continue
    fi

    # ignore the templates directory
    if [ "$familyName" = "_templates" ]; then
        continue
    fi

    # iterate over files in the directory
    for f in $(ls ${entry}); do
        fileName="${f%*.txt}"       # basename of the model specification file
        filePath="${entry}/${f}"    # path to the model specification file
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

# restore previous value of stack limit
ulimit -s "$prevStackLim"
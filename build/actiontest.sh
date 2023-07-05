#!/bin/bash

ulimit -s 'unlimited'

EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
EXEC_FILE='./stv'

LOG_FILE="../output.log"     

date +"%c" > $LOG_FILE

for ((i=1;i<=3;i++)); do
    sed -i "7s/.*/MODEL_ID=${i}/" ./config.txt
    
    echo -e "Model_id = ${i}" >> $LOG_FILE
    res=$($EXEC_TIME $EXEC_FILE --mode 0 2>&1>/dev/null)
    echo -e "mode 0: $res" >> $LOG_FILE
    res=$($EXEC_TIME $EXEC_FILE --mode 1 2>&1>/dev/null)
    echo -e "mode 1: $res" >> $LOG_FILE
    res=$($EXEC_TIME $EXEC_FILE --mode 2 2>&1>/dev/null)
    echo -e "mode 2: $res" >> $LOG_FILE
done


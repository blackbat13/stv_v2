#!/bin/bash

# Repeat the tests ($ARG1 ?? 10) times
REPS=${1:-10}

EXEC_FILE=./sample_test
CSV_FILE='wyniki.csv'
LOG_FILE='mojlog.txt'

# Clear logfile and results
date +"%c" > $LOG_FILE
echo -e "Suite" > $CSV_FILE

echo -e "Running ${EXEC_FILE} for ${REPS} times..."
# Main loop
for (( i=1; i<=$REPS; i++ )); do
    # append column with results for i-th iteration
    sed -i "1s/.*/&\ttime_${i}/" $CSV_FILE
    
    # run the test file
    res=$( $EXEC_FILE 2>&1 | tee -a $LOG_FILE )
    
    # n-th row (of a test suite), where the results will be appended
    ctr=2

    # parse test suite results
    echo "$res" | head -n -3 | while read -r a; do 
        # filter the total results only
        if [[ $a =~ "ms total" ]]; then
            # append results to existing row (if any)
            if test $i -gt 1; then
                entry=$(sed -r 's/[^0-9]+[0-9]+ tests from [^0-9[:space:]]+[^0-9]*([0-9]+).*/\t\1/'<<<"$a")
                sed -i "${ctr}s/.*/&${entry}/" $CSV_FILE
            else
                entry=$(sed -r 's/[^0-9]+[0-9]+ tests from ([^0-9[:space:]]+)[^0-9]*([0-9]+).*/\1\t\2/'<<<"$a")
                echo -e "$entry" >> $CSV_FILE
            fi
            ctr=$((ctr+1))
        fi
    done

    # parse total results
    REG_TOTAL='(\d+)(?= ms total)'
    total_time=$(echo "$res" | tail -n 2 | grep -Po "$REG_TOTAL")
    
    # if this is follow-up iteration, then append to existing row
    if test $i -gt 1; then
        # substitute last line (by appending the new total_time value)
        sed -i "$ s/.*/&\t${total_time}/" $CSV_FILE
    else
        echo -e "Total\t${total_time}" >> $CSV_FILE
    fi

    echo -e "\titeration $i completed;"
done



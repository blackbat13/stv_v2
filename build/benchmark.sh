#!/bin/bash

# Repeat the tests ($ARG1 ?? 10) times
REPS=${1:-10}

EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
EXEC_FILE='./stv'

CSV_FILE='results.csv'
LOG_FILE='mojlog.txt'

TNONCE=$(date +'%Y-%m-%d-%H-%M-%S')
RESULTS="$TNONCE.txt"

func()
{
    echo "see $1"
}

# iterate over files from catalog
# 
for entry in $(ls -d ../examples/*);do
    familyName=${entry##*/}         # directory name 
    # echo $familyName
    for f in $(ls ${entry}); do
        fileName="${f%*.txt}"       # basename of the model
        filePath="${entry}/${f}"
        # echo "$fileName"
        func $filePath
    done
done


echo $ctr
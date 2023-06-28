#!/bin/bash

# Repeat the tests ($ARG1 ?? 10) times
REPS=${1:-10}

EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
EXEC_FILE='./stv'

CSV_FILE='results.csv'
LOG_FILE='mojlog.txt'

TNONCE=$(date +'%Y-%m-%d-%H-%M-%S')
RESULTS="$TNONCE.txt"

# iterate over files from catalog
# tbd
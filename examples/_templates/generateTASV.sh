#!/bin/bash

#----------#
# tasv.ejs #
#----------#
echo -e "Generating family of tasv..."
N_V=(1 5) 
N_C=(1 3) 
OUTDIR='../tasv'
TFILE='./tasv.ejs'
mkdir -p $OUTDIR
for ((i=${N_V[0]};i<=${N_V[1]};i++)); do
    for ((j=${N_C[0]};j<=${N_C[1]};j++)); do
        ofile="$OUTDIR/Voters${i}Candidates${j}.txt"
        npm exec -- ejs $TFILE -i "{\"N_VOTERS\":${i},\"N_CANDIDATES\":${j},\"WITH_FORMULA\":1}" -o $ofile
    done
done
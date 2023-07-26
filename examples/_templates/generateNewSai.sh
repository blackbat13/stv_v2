#!/bin/bash

N_AGENTS=2 #number of agents
K=50 #progress made in learning phase, agent stops learning when reaches 100
QUALITY="[30, 60]" #start quality of agent with id i+1
A_TRUSTS_B="[[2], [1]]" #agent with id i+1, where i is the first coordinate of the array, trusts everyone with id j in the subarray

OUTDIR='../newSai'
mkdir -p $OUTDIR
ofile="$OUTDIR/NewSai${N_AGENTS}Agents${K}Progress.txt"
npm exec -- ejs newSai.ejs -i "{\"N_AGENTS\":${N_AGENTS},\"K\":${K},\"QUALITY\":${QUALITY},\"A_TRUSTS_B\":${A_TRUSTS_B},\"WITH_FORMULA\":1}" -o $ofile
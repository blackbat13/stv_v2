#!/bin/bash
# 
# ================================================
# Generates a family of models from the template
# given a finite domain parameter/configuration
# values forwarded to the template engine
# 
# Note that currently the domains are hard-coded  
# (such behaviour could be changed, and an option
#  to use domains from argv might be added later)
# ================================================

#------------#
# trains.ejs #
#------------#
N_TRAINS_DOMAIN=(1 15) 
OUTDIR='../trains'
mkdir -p $OUTDIR
for ((i=${N_TRAINS_DOMAIN[0]};i<=${N_TRAINS_DOMAIN[1]};i++)); do
    ofile="$OUTDIR/Trains${i}Controller1.txt"
    npm exec -- ejs trains.ejs -i "{\"N_TRAINS\":$i,\"WITH_FORMULA\":1}" -o $ofile
done

#-------------------------#
# simpleVoting--optim.ejs #
#-------------------------#
N_VOTERS_DOMAIN=(1 5) 
N_CANDIDATES_DOMAIN=(1 3) 
OUTDIR='../svote'
mkdir -p $OUTDIR
for ((i=${N_VOTERS_DOMAIN[0]};i<=${N_VOTERS_DOMAIN[1]};i++)); do
    for ((j=${N_CANDIDATES_DOMAIN[0]};j<=${N_CANDIDATES_DOMAIN[1]};j++)); do
        ofile="$OUTDIR/Voters${i}Candidates${j}.txt"
        npm exec -- ejs simpleVoting--optim.ejs -i "{\"N_VOTERS\":${i},\"N_CANDIDATES\":${j},\"WITH_FORMULA\":1}" -o $ofile
    done
done

#-----------#
# timer.ejs #
#-----------#
N_TIMERS_DOMAIN=(1 7)
N_TICKS_DOMAIN=(1 10)
OUTDIR='../timers'
mkdir -p $OUTDIR
for ((i=${N_TIMERS_DOMAIN[0]};i<=${N_TIMERS_DOMAIN[1]};i++)); do
    for ((j=${N_TICKS_DOMAIN[0]};j<=${N_TICKS_DOMAIN[1]};j++)); do
        ofile="$OUTDIR/Timers${i}Ticks${j}.txt"
        npm exec -- ejs timer.ejs -i "{\"N_TIMERS\":${i},\"N_TICKS\":${j},\"WITH_FORMULA\":1}" -o $ofile
    done
done

#---------#
# sai.ejs #
#---------#
N_AGENTS=(1 4)
OUTDIR='../sai'
mkdir -p $OUTDIR
for ((i=${N_AGENTS[0]};i<=${N_AGENTS[1]};i++)); do
    ofile="$OUTDIR/SAI${i}Agents.txt"
    npm exec -- ejs sai.ejs -i "{\"N_AGENTS\":${i},\"WITH_FORMULA\":1}" -o $ofile
done

#----------------#
# fetchQuest.ejs #
#----------------#
N_ADVENTURERS=(1 4)
N_DEPTH=(1 3)
OUTDIR='../fetchQuest'
mkdir -p $OUTDIR
for ((i=${N_ADVENTURERS[0]};i<=${N_ADVENTURERS[1]};i++)); do
    for ((j=${N_DEPTH[0]};j<=${N_DEPTH[1]};j++)); do
        ofile="$OUTDIR/FetchQuest${i}Adventurers${j}Depth.txt"
        npm exec -- ejs fetchQuest.ejs -i "{\"N_ADVENTURERS\":${i},\"N_DEPTH\":${j},\"WITH_FORMULA\":1}" -o $ofile
    done
done
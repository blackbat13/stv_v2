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
echo -e "Generating family of Natural Strategy..."
N_VOTERS_DOMAIN=(1 9)
OUTDIR='../naturalStrategy'
mkdir -p $OUTDIR
for ((i=${N_VOTERS_DOMAIN[0]};i<=${N_VOTERS_DOMAIN[1]};i++)); do
    ofile="$OUTDIR/${i}Voters1Coercer.txt"
    npm exec -- ejs naturalStrategy.ejs -i "{\"N_VOTERS\":$i,\"WITH_FORMULA\":1}" -o $ofile
done
#!/bin/bash

# Repeat the tests ($ARG1 ?? 10) times
REPS=${1:-10}

EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
EXEC_FILE='./stv'
CSV_FILE='results.csv'
LOG_FILE='mojlog.txt'

# Clear logfile and results
date +"%c" > $LOG_FILE
echo -e "case\tmem\treal\tuser\tsys\tstates\tstates_T0\ttransitions_T0\tstates_T1\ttransitions_T1" > $CSV_FILE

echo -e "Running ${EXEC_FILE} for ${REPS} times..."

ctr=2
# Main loop
for (( j=6; j<=10; j++ )); do
    for (( k=1; k<=10; k++ )); do
        # casename="Trains${j}Controller1"
        # casename="Voters${j}Candidates${k}Coercer1"
        casename="Timers${j}Ticks${k}"
        
        for (( i=1; i<=$REPS; i++ )); do
            # run the test file
            # $(cd ../src/examples/svote && npm exec -- ejs Simple_voting.ejs -i "{\"N_VOTERS\":$j,\"N_CANDIDATES\":$k,\"WITH_FORMULA\":0}" -o ./Simple_voting.txt && cd $OLDPWD)
            $(cd ../src/examples && npm exec -- ejs timer.ejs -i "{\"N_TIMERS\":$j,\"N_TICKS\":$k,\"WITH_FORMULA\":1}" -o ./svote/Simple_voting.txt && cd $OLDPWD)
            res=$($EXEC_TIME -a --output="$CSV_FILE" $EXEC_FILE)
            echo -e "\titeration $i completed;"
        done
        nstates=$(echo $res | grep -Po '(?<=Number of global states: )(\d+)')
        localV=$(echo $res | grep -Po '(?<=Timer0 local states: )(\d+)')
        localC=$(echo $res | grep -Po '(?<=Timer1 local states: )(\d+)')
        ntransitionsC=$(echo $res | grep -Po '(?<=Timer0 local transitions: )(\d+)')
        ntransitionsV=$(echo $res | grep -Po '(?<=Timer1 local transitions: )(\d+)')
        sed -i "${ctr}s/.*/${casename}\t&\t${nstates}\t${localC}\t${ntransitionsC}\t${localV}\t${ntransitionsV}/" $CSV_FILE
        # echo -e "States=${nstates}" >> $CSV_FILE
        ctr=$((ctr+1))
    done
done


# # Repeat the tests ($ARG1 ?? 10) times
# REPS=${1:-10}

# EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
# EXEC_FILE='./stv'
# CSV_FILE='results.csv'
# LOG_FILE='mojlog.txt'

# # Clear logfile and results
# date +"%c" > $LOG_FILE
# echo -e "case\tmem\treal\tuser\tsys\tstates\tstates_C\ttransitions_C\tstates_V\ttransitions_V" > $CSV_FILE

# echo -e "Running ${EXEC_FILE} for ${REPS} times..."

# ctr=2
# # Main loop
# for (( j=1; j<=4; j++ )); do
#     for (( k=1; k<=4; k++ )); do
#         # casename="Trains${j}Controller1"
#         casename="Voters${j}Candidates${k}Coercer1"
        
#         for (( i=1; i<=$REPS; i++ )); do
#             # run the test file
#             # $(cd ../src/examples/svote && npm exec -- ejs Simple_voting.ejs -i "{\"N_VOTERS\":$j,\"N_CANDIDATES\":$k,\"WITH_FORMULA\":1}" -o ./Simple_voting.txt && cd $OLDPWD)
#             $(cd ../src/examples/svote && npm exec -- ejs Simple_voting_optim.ejs -i "{\"N_VOTERS\":$j,\"N_CANDIDATES\":$k,\"WITH_FORMULA\":1}" -o ./Simple_voting.txt && cd $OLDPWD)
#             res=$($EXEC_TIME -a --output="$CSV_FILE" $EXEC_FILE)
#             echo -e "\titeration $i completed;"
#         done
#         nstates=$(echo $res | grep -Po '(?<=Number of global states: )(\d+)')
#         localV=$(echo $res | grep -Po '(?<=Voter1 local states: )(\d+)')
#         localC=$(echo $res | grep -Po '(?<=Coercer1 local states: )(\d+)')
#         ntransitionsC=$(echo $res | grep -Po '(?<=Coercer1 local transitions: )(\d+)')
#         ntransitionsV=$(echo $res | grep -Po '(?<=Voter1 local transitions: )(\d+)')
#         sed -i "${ctr}s/.*/${casename}\t&\t${nstates}\t${localC}\t${ntransitionsC}\t${localV}\t${ntransitionsV}/" $CSV_FILE
#         # echo -e "States=${nstates}" >> $CSV_FILE
#         ctr=$((ctr+1))
#     done
# done


# # Repeat the tests ($ARG1 ?? 10) times
# REPS=${1:-10}

# EXEC_TIME='/usr/bin/time -f %M\t%e\t%U\t%S'
# EXEC_FILE='./stv'
# CSV_FILE='results.csv'
# LOG_FILE='mojlog.txt'

# # Clear logfile and results
# date +"%c" > $LOG_FILE
# echo -e "case\tmem\treal\tuser\tsys\tstates" > $CSV_FILE

# echo -e "Running ${EXEC_FILE} for ${REPS} times..."

# ctr=2
# # Main loop
# for (( j=1; j<=15; j++ )); do
#     casename="Trains${j}Controller1"
#     # echo -e "Trains${j}Controller1" >> $CSV_FILE
#     for (( i=1; i<=$REPS; i++ )); do
#         # run the test file
#         $(cd ../src/examples/trains && npm exec -- ejs Trains.ejs -i "{\"N_TRAINS\":$j,\"WITH_FORMULA\":1}" -o ./Trains.txt && cd $OLDPWD)
#         res=$($EXEC_TIME -a --output="$CSV_FILE" $EXEC_FILE)
#         echo -e "\titeration $i completed;"
#     done
#     nstates=$(echo $res | grep -Po '(?<=Number of global states: )(\d+)')
#     sed -i "${ctr}s/.*/${casename}\t&\t${nstates}/" $CSV_FILE
#     # echo -e "States=${nstates}" >> $CSV_FILE
#     ctr=$((ctr+1))
# done
COMMAND="../../build/stv -f Voters1Coercer1.txt -m 3"
TIME1=$(/usr/bin/time -f "%e" $1 2>&1 > /dev/null)
echo $TIME1

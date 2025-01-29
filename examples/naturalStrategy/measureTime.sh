FILES=`ls *.txt | sort`
echo "=====[Measuring execution time]====="
for FILE in $FILES; do
	echo "Measuring $FILE..."
	COMMAND1="../../build/stv -f $FILE -m 1"
	COMMAND2="../../build/stv -f $FILE -m 3"
	COMMAND3="../../build/stv -f $FILE -m 3 --NATURAL_STRATEGY"

	TIME1=$(/usr/bin/time -f "%e" $COMMAND1 2>&1 > /dev/null)
    echo "Generating model time: ${TIME1}s"
	
    TIME2=$(/usr/bin/time -f "%e" $COMMAND2 2>&1 > /dev/null)
    RESULT1=$(echo "scale=2; var=$TIME2; var2=$TIME1; var-=var2; if(var<1&&var>-1&&var!=0) print 0; var" | bc)
    echo "Strategy discovery time (normal): ${RESULT1}s"
	
	TIME3=$(/usr/bin/time -f "%e" $COMMAND3 2>&1 > /dev/null)
    RESULT2=$(echo "scale=2; var=$TIME3; var2=$TIME1; var-=var2; if(var<1&&var>-1&&var!=0) print 0; var" | bc)
	echo "Strategy discovery time (natural): ${RESULT2}s" 

	echo "===================================="
done

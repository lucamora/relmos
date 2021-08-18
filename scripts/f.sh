#!/bin/bash
clear
echo "running full test suite"

SUCCESS=0
TOTAL=0
for (( SUITE = 1; SUITE <= 6; SUITE++ )); do
	echo "running suite $SUITE"

	for (( TASK = 1; TASK <= 2; TASK++ )); do
		start=$(date +%s%N)
		DIFF=$(diff <(cat tests/suite$SUITE/batch$SUITE.$TASK.in | ./relmos) tests/suite$SUITE/batch$SUITE.$TASK.py.out)
		end=$(date +%s%N)

		if [ "$DIFF" == "" ]
		then
			SUCCESS=$((SUCCESS+1))
			echo "test $SUITE.$TASK completed ($(((end-start)/1000000)) ms)"
		else
			echo "test $SUITE.$TASK failed ($(((end-start)/1000000)) ms)"
			echo "$DIFF"
			echo ""
		fi
		TOTAL=$((TOTAL+1))
	done
	echo ""
done

echo ""
echo "tests completed $SUCCESS/$TOTAL"
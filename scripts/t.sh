#!/bin/bash
SUITE=$1
TASK=$2

clear
echo "running $SUITE.$TASK test"
cat tests/suite$SUITE/batch$SUITE.$TASK.in | ./relmos > out.txt
#diff <(cat tests/suite$SUITE/batch$SUITE.$TASK.in | ./relmos) tests/suite$SUITE/batch$SUITE.$TASK.py.out
diff out.txt tests/suite$SUITE/batch$SUITE.$TASK.py.out
echo "test $SUITE.$TASK completed"
#!/bin/bash

# Test script that compares the defrag.c output to the 
# expected output.
# @author John Choi


# Check if Makefile exists
if ! [ -e "Makefile" ]
then
	echo "Makefile does not exist!"
	exit 1
fi

# Check if source code exists
if ! [ -e "defrag.c" ]
then
	echo "defrag.c does not exist!"
	exit 2
fi

# Check if necessary input files exist
for i in {1..3}
do
	if ! [ -e "input/disk_frag_$i" ]
	then
		echo "input/disk_frag_$i does not exist!"
		exit 3
	fi
done

# Start testing

make clean
make

FAIL=0

echo
for filenum in {1..3}
do
	echo "Testing Input $filenum"
	echo
	failed=false
	
	# run the program and ignore the output
	./defrag input/disk_frag_$filenum > /dev/null
	if [ $? != 0 ]
	then
		failed=true
		echo "Expected Exit code: 0 but got $?"
	fi

	# run diff
	diff disk_defrag output/disk_defrag_$filenum
	if [ $? != 0 ]
	then
		failed=true
		echo "Diff reported error"
		echo "Try running vbindiff disk_defrag output/disk_defrag_$filenum"
	fi
	
	# increment FAIL if applicable
	if [ $failed == true ]
	then
		echo
		echo "FAILED!"
		FAIL=$(($FAIL + 1))
	else
		echo "PASSED!"
	fi
	echo
done

# Test the invalid file name case
echo "Testing nonexisting input file"
echo

./defrag this-file-doesnt-exist > /dev/null
if [ $? != 1 ]
then
	FAIL=$(($FAIL + 1))
	echo

	echo "Expected Exit code: 1 but got: $?"
	echo "FAILED!"
else
	echo

	echo "PASSED!"
fi
echo

# report testing result
echo
echo "TEST REPORT"
echo

if [ $FAIL == 0 ]
then
	echo "ALL TESTS PASSED"
else
	echo "$FAIL / 4 TESTS FAILED"
	echo "FAILING TESTS!"
fi

exit 0

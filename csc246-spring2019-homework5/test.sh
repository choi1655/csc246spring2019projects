#!/bin/bash

# Test script that automatically tests four possible inputs and error cases.
# Tests ./myls, ./myls -l, ./myls testDir/, ./myls -l testDir/ + errors
# Author: John Choi
# Version: 04052019

usage() {
	echo "Required files: myls.c, Makefile, testDir/"
}

# Check if required files exist
sourcecode="./myls.c"
makefile="./Makefile"
testdir="./testDir/"

if [ ! -f "$sourcecode" ]
then
	echo "$sourcecode does not exist"
	usage
	exit
fi
if [ ! -f "$makefile" ]
then
	echo "$makefile does not exist"
	usage
	exit
fi
if [ ! -d "$testdir" ]
then
	echo "$testdir does not exist"
	usage
	exit
fi
# First, make clean and make
echo "Making..."
make clean
make

echo
echo

passed=0

# Test ./myls
echo "Testing ./myls"

./myls

if [ $? != 0 ]
then
	echo "Test ./myls FAILED"
else
  echo "Test ./myls PASSED"
  passed=$(($passed + 1))
fi
echo

echo "Running valgrind ./myls"
valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./myls
if [ $? != 0 ]
then
	echo "Valgrind ./myls FAILED"
else
  echo "Valgrind ./myls PASSED"
  passed=$(($passed + 1))
fi
echo

echo
echo

# Test ./myls -l
echo "Testing ./myls -l"

./myls -l
if [ $? != 0 ]
then
	echo "Test ./myls -l FAILED"
else
  echo "Test ./myls -l PASSED"
  passed=$(($passed + 1))
fi
echo

echo "Running valgrind ./myls -l"
valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./myls -l
if [ $? != 0 ]
then
	echo "Valgrind ./myls -l FAILED"
else
  echo "Valgrind ./myls -l PASSED"
  passed=$(($passed + 1))
fi
echo

echo
echo

# Test ./myls testDir/
echo "Testing ./myls testDir/"

./myls testDir/
if [ $? != 0 ]
then
	echo "Test ./myls testDir/ FAILED"
else
  echo "Test ./myls testDir/ PASSED"
  passed=$(($passed + 1))
fi
echo

echo "Running valgrind ./myls testDir/"
valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./myls testDir/
if [ $? != 0 ]
then
	echo "Valgrind ./myls testDir/ FAILED"
else
  echo "Valgrind ./myls testDir/ PASSED"
  passed=$(($passed + 1))
fi
echo

echo
echo

# Test ./myls -l testDir/
echo "Testing ./myls -l testDir/"

./myls -l testDir/
if [ $? != 0 ]
then
	echo "Test ./myls -l testDir/ FAILED"
else
  echo "Test ./myls -l testDir/ PASSED"
  passed=$(($passed + 1))
fi
echo

echo "Running valgrind ./myls -l testDir/"
valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./myls -l testDir/
if [ $? != 0 ]
then
	echo "Valgrind ./myls -l testDir/ FAILED"
else
  echo "Valgrind ./myls -l testDir/ PASSED"
  passed=$(($passed + 1))
fi
echo

echo
echo

# Test error cases
echo "Testing ./myls this-directory-doesnt-exist/"

./myls this-directory-doesnt-exist/
if [ $? != 1 ]
then
	echo "Test ./myls this-directory-doesnt-exist FAILED"
else
  echo "Test ./myls this-directory-doesnt-exist PASSED"
  passed=$(($passed + 1))
fi
echo

echo "Running valgrind for ./myls this-directory-doesnt-exist/"
valgrind --error-exitcode=2 --tool=memcheck --leak-check=full ./myls this-directory-doesnt-exist/
if [ $? == 2 ]
then
	echo "Valgrind ./myls this-directory-doesnt-exist/ FAILED"
else
  echo "Valgrind ./myls this-directory-doesnt-exist/ PASSED"
  passed=$(($passed + 1))
fi
echo

echo
echo

echo "Testing ./myls -l this-directory-doesnt-exist/"

./myls -l this-directory-doesnt-exist/
if [ $? != 1 ]
then
	echo "Test ./myls -l this-directory-doesnt-exist/ FAILED"
else
  echo "Test ./myls -l this-directory-doesnt-exist/ PASSED"
  passed=$(($passed + 1))
fi
echo

echo "Running valgrind ./myls -l this-directory-doesnt-exist/"
valgrind --error-exitcode=2 --tool=memcheck --leak-check=full ./myls -l this-directory-doesnt-exist/
if [ $? == 2 ]
then
	echo "Valgrind ./myls -l this-directory-doesnt-exist/ FAILED"
else
  echo "Valgrind ./myls -l this-directory-doesnt-exist/ PASSED"
  passed=$(($passed + 1))
fi
echo

#Print Test result
echo
echo
if [ $passed == 12 ]
then
  echo "All tests PASSED!"
else
  failed=12
  failed=$((12 - $passed))
  echo "$failed out of 12 tests FAILED!"
fi

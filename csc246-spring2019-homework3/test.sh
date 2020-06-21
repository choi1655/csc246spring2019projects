#!/bin/bash

echo "Running input.txt"
echo
for i in {1..8}
do
	echo
	echo
	echo "Thread number: $i"
	./prob_3 $i input.txt
done

echo
echo

echo "Running input2.txt"
echo
for i in {1..8}
do
	echo
	echo
	echo "Thread number: $i"
	./prob_3 $i input2.txt
done


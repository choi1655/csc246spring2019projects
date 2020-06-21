#!/bin/bash

seed=1

python vsfs.py -n 8 -s $seed >| output.txt
diff expected.txt output.txt

while [ $? == 1 ]
do
	seed=$(($seed + 1))
	python vsfs.py -n 8 -s $seed >| output.txt
	diff expected.txt output.txt
done

echo "Seed is $seed"


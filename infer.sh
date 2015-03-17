#!/bin/sh

#TESTCASES=( testcase1 testcase2 testcase3 testcase4 testcase5 )
TESTCASES=( testcase1 )

BENCH=chase-lev-deque-bugfix

for((i=0;i<${#TESTCASES[@]};i++))
do
	#echo "$i: ${TESTCASES[$i]}"

	input=result$i.txt
	output=result$(($i+1)).txt

	echo input:$input
	echo output:$output

	if [ $i == 0 ]; then
		sh run.sh $BENCH/${TESTCASES[$i]}_wildcard -m2 -y -u3 -tSCFENCE &> tmp.txt
	else
		sh run.sh $BENCH/${TESTCASES[$i]}_wildcard -m2 -y -u3 -tSCFENCE -o f$input &> tmp.txt
	fi
		
	firstLine=`grep "Result 0" tmp.txt -n | tail -n 1 | sed -n 's/^\([0-9]*\)[:].*/\1/p'`
	lastLine=`grep "wildcard" tmp.txt -n | tail -n 1 | sed -n 's/^\([0-9]*\)[:].*/\1/p'`
	head tmp.txt -n "$lastLine" | tail  -n +"$firstLine" &> $output

done



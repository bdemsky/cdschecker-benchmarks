#!/bin/bash
#

# Get the directory in which this script is located
BINDIR="${0%/*}"

INPUT=$1

DIR=$2

if [ -z $DIR ]; then
	echo "Usage: `basename $0` [INPUT_FILE]  [DIRECTORY_OF_SOURCE]"
	exit
fi

TMP=$BINDIR/tmp

mkdir -p $TMP

wildcard=($(awk 'BEGIN {IFS="\t ";} /memory_order/ {print $2;}' $INPUT))
memory_order=($(awk 'BEGIN {IFS="\t ";} /memory_order/ {print $4;}' $INPUT))

for f in $DIR/*; do
	echo "Replacing file $f"
	newFile="$BINDIR/tmp/output-`basename $f`"
	cp $f $newFile
	for i in $(seq 0 $((${#wildcard[@]} - 1))) ;do
		old="wildcard(${wildcard[$i]})"
		new="${memory_order[$i]}"
		cmd="sed -i -e 's/$old/$new/g' $newFile"
		echo $cmd
		bash -c "$cmd"
	done

done

#awk 'wildcard

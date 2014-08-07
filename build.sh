#!/bin/bash

CHECKER_DIR=~/model-checker-priv/model-checker-priv

if [ -z $1 ] ; then
	echo "Use default CDS checker directory"
else
	CHECKER_DIR=$1
fi

make CDS_DIR=$CHECKER_DIR

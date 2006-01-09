#!/bin/bash

if test ! $# -eq 1 || test "$1" == "--help" 
then
    echo "This program checks results of run_simulator.sh and run_gdb.sh " 1>&2
    echo "Use: $0 ARCH" 1>&2
    exit 1
fi

ARCH=$1

for I in *.c
do
  TMP=`echo $I | cut -d '.' -f '1 2'`
  diff --brief --report-identical-files ${TMP}.${ARCH}.out data/$TMP.data
done

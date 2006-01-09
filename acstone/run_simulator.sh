#!/bin/bash

SIMULATOR=/home/bruno/iniciacao/sparcv8/sparcv8/sparcv8.x

if test ! $# -eq 1 || test "$1" == "--help" 
then
    echo "This program run the simulator for each program" 1>&2
    echo "Run this command before run the run_gdb.sh script" 1>&2
    echo "Use: $0 ARCH" 1>&2
    exit 1
fi

ARCH=$1


# For each compiled program construct call a simulator
# to server a gdb connection

#Make a loop for each file
for I in `ls *.${ARCH}`
  do
 
  ${SIMULATOR} --load_obj=${I} --gdb

done


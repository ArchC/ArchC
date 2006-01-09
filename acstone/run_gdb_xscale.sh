#!/bin/bash

if test ! $# -eq 1 || test "$1" == "--help" 
then
    echo "This program run the GNU Debugger for each program" 1>&2
    echo "Run this command after run the run_simulator.sh script" 1>&2
    echo "Use: $0 ARCH" 1>&2
    exit 1
fi

ARCH=$1


# For each compiled program construct a gdb's
# command file and call gdb

# Compute the number lines of inicial first commands file
NLCFG=`cat gdb/firstcommands.gdb | wc -l`

#Make a loop for each file
for I in `ls *.${ARCH}`
  do

  NAME=`echo ${I} | cut -f '1 2' -d '.'`
  NL=`cat gdb/${NAME}.gdb | wc -l`
  NLSHOW=`expr ${NL} - 2`

  rm -f ${NAME}.cmd
  cat gdb/firstcommands.gdb > ${NAME}.cmd
  tail -n ${NLSHOW} gdb/${NAME}.gdb >> ${NAME}.cmd

  echo ${ARCH}-elf-gdb ${I} --command=${NAME}.cmd | cut -s -f 2 -d '$' | cut -f 2 -d '=' > ${NAME}.${ARCH}.out
 
  ${ARCH}-elf-gdb ${I} --command=${NAME}.cmd | cut -s -f 2 -d '$' | cut -f 2 -d '=' > ${NAME}.${ARCH}.out

  sleep 3

done


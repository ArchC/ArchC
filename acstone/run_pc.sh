#!/bin/sh

for I in *.x86
do
  NAME=`echo ${I} | cut -f '1 2' -d '.'`
  gdb ${I} --command=gdb/${NAME}.gdb > ${I}.out
done

## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../aclib/ac_decoder -I../aclib/ac_syscall

## yacc options
AM_YFLAGS = -d -p eval_

## The ArchC compiled simulator generation tool 
bin_PROGRAMS = accsim
BUILT_SOURCES = eval.h
accsim_SOURCES = accsim_extras.h accsim.h acsim.h acsim.c eval.y eval.h accsim.c
accsim_LDADD = ../acpp/libacpp.a ../aclib/libarchc.a

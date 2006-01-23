## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../aclib/ac_decoder

## Passing the sysconfdir to the compiler
CPPFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\" @CPPFLAGS@

## The ArchC interpreted behavioral simulator tool
bin_PROGRAMS = acsim
acsim_SOURCES = acsim.h acsim.c
acsim_LDADD = ../acpp/libacpp.a ../aclib/libarchc.a

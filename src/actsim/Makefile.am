## Process this file with automake to produce Makefile.in

## Flags and macros
AM_CPPFLAGS = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/acpp -DSYSCONFDIR=\"$(sysconfdir)\" -DINCLUDEDIR=\"$(includedir)\" -DBINDIR=\"$(bindir)\" -DLIBDIR=\"$(libdir)\" -DACVERSION=\"$(VERSION)\" @CPPFLAGS@

## The ArchC interpreted behavioral simulator tool
bin_PROGRAMS = actsim
actsim_SOURCES = actsim.h actsim.c 
actsim_LDADD = ../acpp/libacpp.la ../aclib/libarchc.la

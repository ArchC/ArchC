## Process this file with automake to produce Makefile.in

## Flags and macros
AM_CPPFLAGS = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/acpp -DSYSCONFDIR=\"$(sysconfdir)\" -DINCLUDEDIR=\"$(pkgincludedir)\" -DBINDIR=\"$(bindir)\" -DLIBDIR=\"$(libdir)\" -DACVERSION=\"$(VERSION)\" @CPPFLAGS@

## The ArchC interpreted behavioral simulator tool
bin_PROGRAMS = acsim
acsim_SOURCES = acsim.h acsim.c 
acsim_LDADD = ../acpp/libacpp.la ../aclib/libarchc.la

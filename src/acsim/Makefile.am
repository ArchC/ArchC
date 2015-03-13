## Process this file with automake to produce Makefile.in

## Flags and macros
AM_CPPFLAGS = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/acpp -DSYSCONFDIR=\"$(sysconfdir)\" -DARCHCDIR=\"$(prefix)\" -DINCLUDEDIR=\"$(includedir)\" -DBINDIR=\"$(bindir)\" -DLIBDIR=\"$(libdir)\" -DACVERSION=\"$(VERSION)\" @CPPFLAGS@

if HLT_SUPPORT
AM_CPPFLAGS += -DHLT_SUPPORT 
endif


## The ArchC interpreted behavioral simulator tool
bin_PROGRAMS = acsim
acsim_SOURCES = acsim.h acsim.c 
acsim_LDADD = ../acpp/libacpp.la ../aclib/libarchc.la

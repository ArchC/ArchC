## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/acpp -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils

##-I$(top_srcdir)/src/aclib/ac_utils

## yacc options
AM_YFLAGS = -d -p eval_

## Passing the sysconfdir to the compiler
AM_CPPFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\" -DARCHCPATH=\"$(prefix)\" -DINCLUDEDIR=\"$(pkgincludedir)\" -DBINDIR=\"$(bindir)\" -DLIBDIR=\"$(libdir)\" -DACVERSION=\"$(VERSION)\" @CPPFLAGS@

## The ArchC compiled simulator generation tool 
bin_PROGRAMS = accsim
BUILT_SOURCES = eval.h
accsim_SOURCES = accsim_extras.h accsim.h acsim.h acsim.c eval.y eval.h accsim.c
accsim_LDADD = ../acpp/libacpp.la ../aclib/libarchc.la

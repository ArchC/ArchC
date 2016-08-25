## Process this file with automake to produce Makefile.in

## Includes
AM_CPPFLAGS = -std=c++11 -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_rtld @SYSTEMC_CFLAGS@


## The ArchC library
noinst_LTLIBRARIES = libacutils.la

## ArchC library includes

include_HEADERS = ac_debug_model.H elf32-tiny.h archc.H ac_utils.H ac_log.H ac_msgbuf.H ac_args.H
libacutils_la_SOURCES = ac_utils.cpp


if HLT_SUPPORT
include_HEADERS +=  ac_hltrace.H
libacutils_la_SOURCES += ac_hltrace.cpp
AM_CPPFLAGS += @LIBELF_CFLAGS@
endif

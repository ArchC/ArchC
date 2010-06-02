## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_rtld -I$(SC_DIR)/include

## The ArchC library
noinst_LTLIBRARIES = libacutils.la

## ArchC library includes
pkginclude_HEADERS = ac_debug_model.H elf32-tiny.h archc.H ac_utils.H ac_log.H ac_msgbuf.H

libacutils_la_SOURCES = ac_utils.cpp

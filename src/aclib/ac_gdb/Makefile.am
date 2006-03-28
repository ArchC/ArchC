## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils

## The ArchC library
noinst_LTLIBRARIES = libacgdb.la

## ArchC library includes
pkginclude_HEADERS = breakpoints.H ac_gdb.H ac_gdb_interface.H

## Adding code to the ArchC library
libacgdb_la_SOURCES = breakpoints.cpp

## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils

## The ArchC library
noinst_LTLIBRARIES = libacstorage.la

## ArchC library includes
pkginclude_HEADERS = ac_cache.H ac_storage.H ac_ptr.H ac_regbank.H ac_inout_if.H ac_sync_reg.H ac_reg.H ac_mem.H ac_cache_if.H ac_memport.H

libacstorage_la_SOURCES = ac_storage.cpp

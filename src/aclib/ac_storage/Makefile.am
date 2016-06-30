## Process this file with automake to produce Makefile.in

## Includes
AM_CPPFLAGS = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils @SYSTEMC_CFLAGS@

## The ArchC library
noinst_LTLIBRARIES = libacstorage.la

## ArchC library includes
include_HEADERS = ac_inout_if.H ac_memport.H ac_ptr.H ac_regbank.H ac_reg.H ac_mem.H ac_sync_reg.H  

#libacstorage_la_SOURCES = ac_mem.cpp ac_cache_trace.cpp
libacstorage_la_SOURCES = ac_mem.cpp

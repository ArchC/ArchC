## Process this file with automake to produce Makefile.in

## Includes
AM_CPPFLAGS = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils @SYSTEMC_CFLAGS@

## The ArchC library
noinst_LTLIBRARIES = libacstorage.la

## ArchC library includes
#include_HEADERS = ac_mem.H ac_memport.H ac_ptr.H ac_inout_if.H ac_regbank.H ac_reg.H ac_storage.H ac_sync_reg.H
include_HEADERS = ac_cache_bhv.H ac_cache.H ac_cache_if.H ac_cache_replacement_policy.H ac_cache_trace.H ac_fifo_replacement_policy.H ac_inout_if.H ac_lru_replacement_policy.H ac_mem.H ac_memport.H ac_plrum_replacement_policy.H  ac_ptr.H ac_random_replacement_policy.H ac_regbank.H ac_reg.H ac_storage.H ac_sync_reg.H ac_cache_power.H Dir.h cacheMem.h cacheBlock.h 

#libacstorage_la_SOURCES = ac_storage.cpp ac_cache_trace.cpp
libacstorage_la_SOURCES = ac_storage.cpp ac_cache_trace.cpp cacheBlock.cpp cacheMem.cpp Dir.cpp

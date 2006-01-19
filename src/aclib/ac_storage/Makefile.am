## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../ac_core -I../ac_decoder -I../ac_encoder -I../ac_gdb -I../ac_storage -I../ac_syscall -I../ac_utils -I$(SC_DIR)/include

## The ArchC library
noinst_LIBRARIES = libacstorage.a

## ArchC library includes
include_HEADERS = ac_cache.H ac_storage.H ac_ptr.H ac_regbank.H ac_inout_if.H ac_reg.H ac_mem.H ac_cache_if.H ac_memport.H

libacstorage_a_SOURCES = ac_storage.cpp

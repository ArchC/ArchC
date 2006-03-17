## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils -I$(TLM_DIR)

## The ArchC library
noinst_LIBRARIES = libactlm.a

## ArchC library includes
pkginclude_HEADERS = ac_tlm_protocol.H ac_tlm_port.H

libactlm_a_SOURCES = ac_tlm_port.cpp

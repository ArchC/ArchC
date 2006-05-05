## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils -I$(TLM_DIR) -I$(SC_DIR)/include/

## The ArchC library
noinst_LTLIBRARIES = libactlm.la

## ArchC library includes
pkginclude_HEADERS = ac_tlm_protocol.H ac_tlm_port.H ac_tlm_intr_port.H ac_intr_handler.H ac_tlm_dev_id.H

libactlm_la_SOURCES = ac_tlm_port.cpp ac_tlm_intr_port.cpp ac_tlm_dev_id.cpp

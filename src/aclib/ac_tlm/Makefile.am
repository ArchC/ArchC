## Process this file with automake to produce Makefile.in

## Includes
AM_CPPFLAGS = -I. -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils @SYSTEMC_CFLAGS@

## The ArchC library
noinst_LTLIBRARIES = libactlm.la

## ArchC library includes
include_HEADERS = ac_tlm_protocol.H ac_tlm_port.H ac_tlm_intr_port.H ac_intr_handler.H ac_tlm_dev_id.H tlm_payload_dir_extension.h

#libactlm_la_SOURCES = ac_tlm_port.cpp ac_tlm_intr_port.cpp ac_tlm_dev_id.cpp 
libactlm_la_SOURCES = ac_tlm_port.cpp ac_tlm2_port.cpp ac_tlm2_nb_port.cpp ac_tlm_intr_port.cpp ac_tlm_dev_id.cpp

if HAVE_TLM2
AM_CPPFLAGS += @TLM2_CFLAGS@
include_HEADERS += ac_tlm2_port.H ac_tlm2_nb_port.H ac_tlm2_intr_port.H ac_tlm2_payload.H
libactlm_la_SOURCES +=  ac_tlm2_port.cpp ac_tlm2_nb_port.cpp ac_tlm2_intr_port.cpp
else
AM_CPPFLAGS += @TLM_CFLAGS@
endif


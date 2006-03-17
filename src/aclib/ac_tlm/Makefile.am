## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../ac_core -I../ac_decoder -I../ac_encoder -I../ac_gdb -I../ac_storage -I../ac_syscall -I../ac_utils -I$(SC_DIR)/include -I$(TLM_DIR)

## The ArchC library
noinst_LIBRARIES = libactlm.a

## ArchC library includes
pkginclude_HEADERS = ac_tlm_protocol.H ac_tlm_port.H

libactlm_a_SOURCES = ac_tlm_port.cpp

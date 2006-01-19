## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../ac_core -I../ac_decoder -I../ac_encoder -I../ac_gdb -I../ac_storage -I../ac_syscall -I../ac_utils -I$(SC_DIR)/include

## The ArchC library
noinst_LIBRARIES = libacutils.a

## ArchC library includes
include_HEADERS = ac_debug_model.H elf32-tiny.h archc.H ac_utils.H ac_log.H ac_msgbuf.H ac_sto_stats.H

libacutils_a_SOURCES = ac_utils.cpp

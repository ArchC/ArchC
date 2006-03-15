## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../ac_core -I../ac_decoder -I../ac_encoder -I../ac_gdb -I../ac_storage -I../ac_syscall -I../ac_utils -I$(SC_DIR)/include

## The ArchC library
noinst_LIBRARIES = libacgdb.a

## ArchC library includes
include_HEADERS = breakpoints.H ac_gdb.H ac_gdb_interface.H

## Adding code to the ArchC library
libacgdb_a_SOURCES = breakpoints.cpp

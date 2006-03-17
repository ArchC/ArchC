## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../ac_core -I../ac_decoder -I../ac_encoder -I../ac_gdb -I../ac_storage -I../ac_syscall -I../ac_utils -I$(SC_DIR)/include

## The ArchC library
noinst_LIBRARIES = libacdecoder.a

## ArchC library includes
pkginclude_HEADERS = ac_decoder_rt.H ac_decoder.h

libacdecoder_a_SOURCES = ac_decoder.c ac_decoder_rt.cpp

## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../ac_core -I../ac_decoder -I../ac_encoder -I../ac_gdb -I../ac_storage -I../ac_syscall -I../ac_utils -I$(SC_DIR)/include

## The ArchC library
noinst_LIBRARIES = libaccore.a

## ArchC library includes
pkginclude_HEADERS = ac_arch_dec_if.H ac_arch_ref.H ac_instr_info.H ac_arch.H ac_instr.H ac_sighandlers.H ac_module.H

## Adding code to the ArchC library
libaccore_a_SOURCES = ac_module.cpp

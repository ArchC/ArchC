## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils -I$(SC_DIR)/include

## The ArchC library
noinst_LTLIBRARIES = libaccore.la

## ArchC library includes
pkginclude_HEADERS = ac_arch_dec_if.H ac_arch_ref.H ac_instr_info.H ac_arch.H ac_instr.H ac_sighandlers.H ac_module.H ac_stage.H

## Adding code to the ArchC library
libaccore_la_SOURCES = ac_module.cpp ac_sighandlers.cpp

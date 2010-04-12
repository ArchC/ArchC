## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_rtld -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils -I$(SC_DIR)/include/

## The ArchC library
noinst_LTLIBRARIES = libacrtld.la

## ArchC library includes
pkginclude_HEADERS = ac_rtld.H memmap.H ac_rtld_config.H

libacrtld_la_SOURCES = ac_rtld.cpp dynamic_info.cpp dynamic_relocations.cpp dynamic_symbol_table.cpp link_node.cpp memmap.cpp version_definitions.cpp version_needed.cpp ac_rtld_config.cpp


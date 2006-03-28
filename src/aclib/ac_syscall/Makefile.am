## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_storage -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_utils

## The ArchC library
noinst_LTLIBRARIES = libacsyscall.la

## ArchC library includes
pkginclude_HEADERS = ac_syscall_codes.h ac_syscall.H ac_syscall.def

libacsyscall_la_SOURCES = ac_syscall.cpp

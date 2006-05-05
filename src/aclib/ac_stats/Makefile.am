## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/aclib/ac_gdb -I$(top_srcdir)/src/aclib/ac_core -I$(top_srcdir)/src/aclib/ac_syscall -I$(top_srcdir)/src/aclib/ac_utils

## The ArchC library
noinst_LTLIBRARIES = libacstats.la

## ArchC library includes
pkginclude_HEADERS = ac_basic_stats.H ac_instruction_stats.H ac_printable_stats.H ac_processor_stats.H ac_stats_base.H ac_stats.H

libacstats_la_SOURCES = ac_stats_base.cpp

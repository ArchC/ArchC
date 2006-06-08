## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder

## yacc options
AM_YFLAGS = -d

## The ArchC parser/preprocessor library 
noinst_LTLIBRARIES = libacpp.la
BUILT_SOURCES = archc_grammar.h
libacpp_la_SOURCES = archc_lex.l archc_grammar.y archc_grammar.h acpp.c acpp.h core_actions.c core_actions.h bj_hash.c bj_hash.h asm_actions.c asm_actions.h ac_tools_common.h

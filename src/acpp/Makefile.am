## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/acsim

## yacc options
AM_YFLAGS = -d

## The ArchC parser/preprocessor library 
noinst_LIBRARIES = libacpp.a
BUILT_SOURCES = archc_grammar.h
libacpp_a_SOURCES = archc_lex.l archc_grammar.y archc_grammar.h acpp.c acpp.h core_actions.c core_actions.h asm_actions.c asm_actions.h

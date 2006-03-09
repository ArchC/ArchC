## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../aclib/ac_decoder -I../acsim

## yacc options
AM_YFLAGS = -d

## The ArchC parser/preprocessor library 
lib_LIBRARIES = libacpp.a
BUILT_SOURCES = archc_grammar.h
libacpp_a_SOURCES = archc_lex.l archc_grammar.y archc_grammar.h acpp.c acpp.h core_actions.c core_actions.h asm_actions.c asm_actions.h

## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../aclib/ac_decoder -I../acsim

## yacc options
AM_YFLAGS = -d

## The ArchC parser/preprocessor library 
lib_LIBRARIES = libacpp.a
BUILT_SOURCES = archc_parser.h
libacpp_a_SOURCES = parser.acasm.h archc_lex.l archc_parser.y archc_parser.h acpp.h acpp.c parser.acasm.c

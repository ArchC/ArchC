## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I../ac_core/ -I../ac_decoder/ -I../ac_encoder/ -I..ac_gdb/ -I../ac_storage/ -I../ac_syscall/ -I../ac_utils/ -I/home/thiago/SystemC/include/

## The ArchC library
noinst_LIBRARIES = libacsyscall.a

## ArchC library includes
include_HEADERS = ac_syscall_codes.h ac_syscall.H ac_syscall.def

libacsyscall_a_SOURCES = ac_syscall.cpp

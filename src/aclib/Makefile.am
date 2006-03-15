## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I./ac_core -I./ac_decoder -I./ac_encoder -I./ac_gdb -I./ac_storage -I./ac_syscall -I./ac_utils -I$(SC_DIR)/include

## Subdirectories
SUBDIRS = ac_core ac_decoder ac_encoder ac_gdb ac_storage ac_syscall $(MAYBE_TLM) ac_utils

## The ArchC libraries
lib_LIBRARIES = libarchc.a
# libarchc.a has no sources, they've already been compiled in the subdirs.
libarchc_a_SOURCES =
libarchc_a_LIBADD = ac_core/ac_module.o ac_decoder/ac_decoder.o ac_decoder/ac_decoder_rt.o ac_storage/ac_storage.o ac_syscall/ac_syscall.o $(MAYBE_TLM_OBJ) ac_utils/ac_utils.o ac_gdb/breakpoints.o

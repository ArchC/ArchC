## Process this file with automake to produce Makefile.in

## Includes
INCLUDES = -I. -I.. -I./ac_core -I./ac_decoder -I./ac_gdb -I./ac_rtld -I./ac_storage -I ./ac_stats -I./ac_syscall -I./ac_utils -I$(SC_DIR)/include


if TLM_SUPPORT
  TLM_MODULE = ac_tlm
  TLM_LIB = ac_tlm/libactlm.la
else
  TLM_MODULE =
  TLM_LIB = 
endif


## Subdirectories
SUBDIRS = ac_core ac_decoder ac_gdb ac_rtld ac_storage ac_stats ac_syscall $(TLM_MODULE) ac_utils

## The ArchC libraries
lib_LTLIBRARIES = libarchc.la
# libarchc.a has no sources, they've already been compiled in the subdirs.
libarchc_la_SOURCES =
libarchc_la_LIBADD = ac_core/libaccore.la ac_decoder/libacdecoder.la ac_rtld/libacrtld.la ac_storage/libacstorage.la ac_stats/libacstats.la ac_syscall/libacsyscall.la $(TLM_LIB) ac_utils/libacutils.la ac_gdb/libacgdb.la

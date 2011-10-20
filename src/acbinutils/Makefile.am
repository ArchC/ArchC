## Process this file with automake to produce Makefile.in

#SUBDIRS = binutils gdb

EXTRA_DIST = acbingen.sh.in \
             binutils \
             gdb

## Includes
INCLUDES = -I. -I$(top_srcdir)/src/replace -I$(top_srcdir)/src/aclib/ac_decoder -I$(top_srcdir)/src/acpp -I$(top_srcdir)/src/acsim -I$(top_srcdir)/src/aclib/ac_utils

AM_CFLAGS = -DACVERSION=\"$(VERSION)\"

BINUTILS_ROOT = binutils

BINUTILS_TREE = \
	$(BINUTILS_ROOT)/include \
	$(BINUTILS_ROOT)/include/elf \
	$(BINUTILS_ROOT)/include/opcode \
	$(BINUTILS_ROOT)/bfd \
	$(BINUTILS_ROOT)/opcodes \
	$(BINUTILS_ROOT)/gas \
	$(BINUTILS_ROOT)/gas/config \
	$(BINUTILS_ROOT)/ld \
	$(BINUTILS_ROOT)/ld/emulparams

FILES_TO_PATCH = \
	bfd/archures.c \
	bfd/Makefile.in \
	bfd/bfd-in2.h \
	bfd/config.bfd \
	bfd/configure \
	bfd/targets.c \
	config.sub \
	gas/configure.tgt \
	gas/Makefile.in \
	opcodes/configure \
	opcodes/Makefile.in \
	opcodes/disassemble.c \
	ld/configure.tgt \
	ld/Makefile.in \
  	include/dis-asm.h

FILES_TO_COPY = \
	gas/config/tc-xxxxx.c \
	gas/config/tc-xxxxx.h \
	opcodes/xxxxx-opc.c \
	opcodes/xxxxx-dis.c \
	include/opcode/xxxxx.h \
	include/elf/xxxxx.h \
	bfd/elf32-xxxxx.c \
	bfd/cpu-xxxxx.c \
	ld/emulparams/xxxxxelf.sh \
	include/share-xxxxx.h \
	bfd/share-xxxxx.c 

GDB_ROOT = gdb

GDB_TREE = \
	$(GDB_ROOT)/include \
	$(GDB_ROOT)/include/elf \
	$(GDB_ROOT)/include/opcode \
	$(GDB_ROOT)/bfd \
	$(GDB_ROOT)/opcodes \
	$(GDB_ROOT)/gdb \
	$(GDB_ROOT)/gdb/config/xxxxx

FILES_TO_PATCH_GDB = \
	bfd/archures.c \
	bfd/Makefile.in \
	bfd/bfd-in2.h \
	bfd/config.bfd \
	bfd/configure \
	bfd/targets.c \
	config.sub \
	gdb/configure.host \
	gdb/configure.tgt \
	gdb/Makefile.in \
	opcodes/configure \
	opcodes/Makefile.in \
	opcodes/disassemble.c \
	include/dis-asm.h

FILES_TO_COPY_GDB = \
	gdb/xxxxx-tdep.c \
	gdb/config/xxxxx/xxxxx.mt \
	opcodes/xxxxx-opc.c \
	opcodes/xxxxx-dis.c \
	include/opcode/xxxxx.h \
	include/elf/xxxxx.h \
	bfd/elf32-xxxxx.c \
	bfd/cpu-xxxxx.c \
	include/share-xxxxx.h \
	bfd/share-xxxxx.c 

bin_PROGRAMS = bmdsfg acrelconvert
bmdsfg_SOURCES = main.c utils.c utils.h opcodes.c opcodes.h bfd.c bfd.h gas.c gas.h 
bmdsfg_LDADD = ../acpp/libacpp.la ../replace/libreplace.la
acrelconvert_SOURCES = acrelconvert.c

edit = sed \
	-e 's,@pkgdatadir\@,$(pkgdatadir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@bindir\@,${bindir},g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@patchfiles\@,$(FILES_TO_PATCH),g' \
	-e 's,@copyfiles\@,$(FILES_TO_COPY),g' \
	-e 's,@patchfilesgdb\@,$(FILES_TO_PATCH_GDB),g' \
	-e 's,@copyfilesgdb\@,$(FILES_TO_COPY_GDB),g' \
	-e 's,@VERSION\@,$(VERSION),g'

bin_SCRIPTS = acbingen.sh

acbingen.sh: Makefile $(srcdir)/acbingen.sh.in
	$(edit) $(srcdir)/acbingen.sh.in > acbingen.sh

install-data-hook:
	for directory in $(BINUTILS_ROOT) $(BINUTILS_TREE); do \
	  mkdir -p $(pkgdatadir)/$${directory}; \
	done;	
	for file in $(FILES_TO_PATCH); do \
	  cp -f $(srcdir)/$(BINUTILS_ROOT)/$${file}.sed $(pkgdatadir)/$(BINUTILS_ROOT)/$${file}.sed; \
	done;
	for file in $(FILES_TO_COPY); do \
	  cp -f $(srcdir)/$(BINUTILS_ROOT)/$${file} $(pkgdatadir)/$(BINUTILS_ROOT)/$${file}; \
	done
	for directory in $(GDB_ROOT) $(GDB_TREE); do \
	  mkdir -p $(pkgdatadir)/$${directory}; \
	done;	
	for file in $(FILES_TO_PATCH_GDB); do \
	  cp -f $(srcdir)/$(GDB_ROOT)/$${file}.sed $(pkgdatadir)/$(GDB_ROOT)/$${file}.sed; \
	done;
	for file in $(FILES_TO_COPY_GDB); do \
	  cp -f $(srcdir)/$(GDB_ROOT)/$${file} $(pkgdatadir)/$(GDB_ROOT)/$${file}; \
	done

uninstall-hook:
	rm -rf $(pkgdatadir)/$(BINUTILS_ROOT)
	rm -rf $(pkgdatadir)/$(GDB_ROOT)
	rm acbingen.sh

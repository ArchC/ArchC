# Utilities
FLEX   := flex
BISON  := bison
CC     := gcc-3.3
CXX    := g++-3.3
AR     := ar
RANLIB := ranlib
LN     := ln -s

# Top directories
topdir     := .
srcdir     := $(topdir)/src
libdir     := $(topdir)/lib
bindir     := $(topdir)/bin
includedir := $(topdir)/include
examplesdir:= $(topdir)/examples
modelsdir  := $(topdir)/models
configdir  := $(topdir)/config
specialdirs:= $(bindir) $(examplesdir) $(includedir) $(modelsdir) $(libdir)

# SystemC
SYSTEMC := $(topdir)/systemc

# ArchC libs
aclibdir   := $(srcdir)/aclib
coredir    := $(aclibdir)/ac_core
decoderdir := $(aclibdir)/ac_decoder
encoderdir := $(aclibdir)/ac_encoder
storagedir := $(aclibdir)/ac_storage
syscalldir := $(aclibdir)/ac_syscall
tlmdir     := $(aclibdir)/ac_tlm
utilsdir   := $(aclibdir)/ac_utils
gdbdir     := $(aclibdir)/ac_gdb

# ArchC tools
acppdir    := $(srcdir)/acpp
acsimdir   := $(srcdir)/acsim
accsimdir  := $(srcdir)/accsim
acasmdir   := $(srcdir)/acasm

# Initialize variables for tools/libs
set_env := set_env
libs    :=
tools   :=
exports :=
clean   := $(libdir) $(includedir)
modules := $(coredir) $(decoderdir) $(encoderdir) $(storagedir) $(syscalldir) \
           $(utilsdir) $(tlmdir) $(acppdir) $(acsimdir) $(accsimdir) \
	   $(gdbdir) $(acasmdir)
deprecated:= $(topdir)/acpp


all: archc

include $(patsubst %,%/Makefile.inc,$(modules))

archc: $(specialdirs) $(exports) $(libs) $(tools) \
       $(topdir)/config/archc.conf $(deprecated)

$(specialdirs):
	mkdir -p $@

$(topdir)/acpp:
	$(LN) -s $(bindir)/acsim $@

$(tools): $(libs)


doc: Doxyfile
	doxygen Doxyfile

clean:
	rm -rf $(clean) $(libs) $(tools) $(exports) $(deprecated)

distclean: clean
	rm -rf $(configdir) systemc

config/archc.conf:
	@./install.sh

.PHONY: all doc clean distclean

.SILENT:

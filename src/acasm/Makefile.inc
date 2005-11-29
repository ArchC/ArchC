acasmscriptdir  := $(acasmdir)/scripts
acasmspecialdir := $(includedir)/acasm
binutilsdir     := $(acasmspecialdir)/binutils
binutilsgasdir  := $(binutilsdir)/gas
binutilsbfddir  := $(binutilsdir)/bfd
binutilsopcdir  := $(binutilsdir)/opcodes
binutilstree    := $(binutilsgasdir) $(binutilsbfddir) $(binutilsopcdir)

specialdirs += $(acasmspecialdir) $(binutilsdir) $(binutilstree)
tools   += $(bindir)/acasm
exports += $(acasmspecialdir)/tc-templ.c
exports += $(bindir)/asmgen.sh
exports += $(binutilsdir)/config.sub.sed
exports += $(binutilsbfddir)/archures.c.sed
exports += $(binutilsbfddir)/bfd-in2.h.sed
exports += $(binutilsbfddir)/config.bfd.sed
exports += $(binutilsbfddir)/configure.sed
exports += $(binutilsbfddir)/Makefile.in.sed
exports += $(binutilsbfddir)/targets.c.sed
exports += $(binutilsgasdir)/configure.tgt.sed
exports += $(binutilsgasdir)/Makefile.in.sed              
exports += $(binutilsopcdir)/configure.sed
exports += $(binutilsopcdir)/Makefile.in.sed


$(bindir)/acasm: $(acasmdir)/acasm.c $(acasmdir)/acasm.h $(includedir)/ac_decoder.h $(acppdir)/parser.acasm.h $(libdir)/libacpp.a
	@echo "Building ArchC Assembler Generator"
	$(CC) -g -I$(includedir) -I$(acppdir) -o $@ $< -L$(libdir) -lacpp

$(acasmspecialdir)/%: $(acasmscriptdir)/%
	cp $^ $@

$(bindir)/%: $(acasmscriptdir)/%
	cp $^ $@

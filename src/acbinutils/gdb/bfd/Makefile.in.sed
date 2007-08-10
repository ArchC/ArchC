/ALL_MACHINES = \\/ a\
	cpu-xxxxx.lo \\

/ALL_MACHINES_CFILES = \\/ a\
	cpu-xxxxx.c \\

/BFD32_BACKENDS = \\/ a\
	elf32-xxxxx.lo \\\
   share-xxxxx.lo \\

/BFD32_BACKENDS_CFILES = \\/ a\
	elf32-xxxxx.c \\\
   share-xxxxx.c \\

/cpu-alpha.lo:/ i\
cpu-xxxxx.lo: cpu-xxxxx.c $(INCDIR)/filenames.h $(INCDIR)/hashtab.h

/aout-adobe.lo:/ i\
elf32-xxxxx.lo: elf32-xxxxx.c $(INCDIR)/filenames.h $(INCDIR)/hashtab.h \\\
  $(INCDIR)/bfdlink.h genlink.h elf-bfd.h $(INCDIR)/elf/common.h \\\
  $(INCDIR)/elf/internal.h $(INCDIR)/elf/external.h \\\
  $(INCDIR)/elf/reloc-macros.h $(INCDIR)/elf/xxxxx.h elf32-target.h $(INCDIR)/share-xxxxx.h \
share-xxxxx.lo: share-xxxxx.c $(INCDIR)/share-xxxxx.h

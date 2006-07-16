/CPU_TYPES = \\/ a\
	xxxxx \\

/TARGET_CPU_CFILES = \\/ a\
	config/tc-xxxxx.c \\

/TARGET_CPU_HFILES = \\/ a\
	config/tc-xxxxx.h \\

/DEPTC_a29k_aout =/ i\
DEPTC_xxxxx_elf = $(INCDIR)/symcat.h $(srcdir)/config/obj-elf.h \\\
  $(BFDDIR)/elf-bfd.h $(INCDIR)/elf/common.h $(INCDIR)/elf/internal.h \\\
  $(INCDIR)/elf/external.h $(INCDIR)/bfdlink.h $(srcdir)/config/tc-xxxxx.h \\\
  subsegs.h $(INCDIR)/obstack.h $(INCDIR)/safe-ctype.h \\\
  $(INCDIR)/opcode/xxxxx.h $(INCDIR)/elf/xxxxx.h $(INCDIR)/share-xxxxx.h

/DEPOBJ_a29k_aout =/ i\
DEPOBJ_xxxxx_elf = $(INCDIR)/symcat.h $(srcdir)/config/obj-elf.h \\\
  $(BFDDIR)/elf-bfd.h $(INCDIR)/elf/common.h $(INCDIR)/elf/internal.h \\\
  $(INCDIR)/elf/external.h $(INCDIR)/bfdlink.h $(srcdir)/config/tc-xxxxx.h \\\
  $(INCDIR)/safe-ctype.h subsegs.h $(INCDIR)/obstack.h \\\
  struc-symbol.h $(INCDIR)/elf/xxxxx.h 

/DEP_a29k_aout =/ i\
DEP_xxxxx_elf = $(srcdir)/config/obj-elf.h $(INCDIR)/symcat.h \\\
  $(BFDDIR)/elf-bfd.h $(INCDIR)/elf/common.h $(INCDIR)/elf/internal.h \\\
  $(INCDIR)/elf/external.h $(INCDIR)/bfdlink.h $(srcdir)/config/tc-xxxxx.h

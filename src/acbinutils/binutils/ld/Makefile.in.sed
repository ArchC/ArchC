/ALL_EMULATIONS = \\/ a\
	exxxxxelf.o \\

/ea29k.c:/ i\
exxxxxelf.c: $(srcdir)/emulparams/xxxxxelf.sh \\\
  $(srcdir)/emultempl/elf32.em $(srcdir)/scripttempl/elf.sc ${GEN_DEPENDS} \
	${GENSCRIPTS} xxxxxelf "$(tdir_xxxxxelf)"

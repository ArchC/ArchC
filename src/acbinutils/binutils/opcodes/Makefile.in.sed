/CFILES = \\/ a\
	xxxxx-opc.c \\\
	xxxxx-dis.c \\

/ALL_MACHINES = \\/ a\
	xxxxx-opc.lo \\\
   xxxxx-dis.lo \\

/a29k-dis.lo:/ i\
xxxxx-opc.lo: xxxxx-opc.c sysdep.h config.h $(INCDIR)/ansidecl.h \\\
  $(INCDIR)/opcode/xxxxx.h \
xxxxx-dis.lo: xxxxx-dis.c sysdep.h config.h $(INCDIR)/ansidecl.h \\\
  $(INCDIR)/dis-asm.h $(BFD_H) $(INCDIR)/symcat.h $(INCDIR)/opcode/xxxxx.h \\\
  $(INCDIR)/share-xxxxx.h

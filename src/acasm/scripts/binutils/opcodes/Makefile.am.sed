/CFILES = \\/ a\
	xxxxx-opc.c \\

/ALL_MACHINES = \\/ a\
	xxxxx-opc.lo \\

/a29k-dis.lo:/ i\
xxxxx-opc.lo: xxxxx-opc.c sysdep.h config.h $(INCDIR)/ansidecl.h \\\
  $(INCDIR)/opcode/xxxxx.h

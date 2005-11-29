libs  += $(libdir)/libacpp.a

clean += $(acppdir)/lex.yy.c \
         $(acppdir)/lex.yy.o \
         $(acppdir)/archc.parser.c \
         $(acppdir)/archc.parser.h \
         $(acppdir)/archc.parser.o \
	 $(acppdir)/acpp.o \
	 $(acppdir)/parser.acasm.o

$(acppdir)/lex.yy.o: $(acppdir)/lex.yy.c $(acppdir)/archc.parser.h
$(acppdir)/lex.yy.c: $(acppdir)/archc.lex
	@echo "Compiling ArchC Lexer"
	$(FLEX) -o$@ $<


$(acppdir)/archc.parser.o: CFLAGS += -I$(acsimdir) -I$(includedir)
$(acppdir)/archc.parser.o: $(acppdir)/archc.parser.c $(acsimdir)/acsim.h 
$(acppdir)/archc.parser.c $(acppdir)/archc.parser.h: $(acppdir)/archc.bison $(acppdir)/parser.acasm.h
	@echo "Compiling ArchC Parser"
	$(BISON) --defines -o $(acppdir)/archc.parser.c $<


$(acppdir)/acpp.o: $(acppdir)/acpp.c $(acppdir)/acpp.h
	@echo "Compiling ArchC Pre-processor"
	$(CC) -g -c -o $@ $<

$(acppdir)/parser.acasm.o: $(acppdir)/parser.acasm.c $(acppdir)/parser.acasm.h $(includedir)/ac_decoder.h
	$(CC) -g -c -I$(includedir) -o $@ $<			   


$(libdir)/libacpp.a: $(acppdir)/acpp.o $(acppdir)/lex.yy.o $(acppdir)/archc.parser.o $(libdir)/ac_decoder.o $(acppdir)/parser.acasm.o
	@echo "Building ArchC Pre-processor Library"
	$(AR) rcs $@ $^
	$(RANLIB) $@

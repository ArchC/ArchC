tools += $(bindir)/accsim-bin
exports += $(includedir)/ac_utils.H
clean += $(accsimdir)/eval.parse.c $(accsimdir)/eval.parse.h $(bindir)/accsim

$(bindir)/accsim-bin: $(accsimdir)/accsim.c $(accsimdir)/accsim.h $(accsimdir)/acsim.c $(accsimdir)/acsim.h $(includedir)/ac_decoder.h $(accsimdir)/eval.y 
	@echo "Building ArchC Compiled Simulator Generator"
	@$(BISON) -d -p eval_ $(accsimdir)/eval.y -o $(accsimdir)/eval.parse.c
	$(CC) -g -I$(includedir) -o $@ $< $(accsimdir)/acsim.c $(accsimdir)/eval.parse.c -L$(libdir) -lacpp
	rm -f $(bindir)/accsim
	$(LN) $(set_env) $(bindir)/accsim

$(includedir)/%: $(accsimdir)/%
	cp $^ $@

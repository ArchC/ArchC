tools += $(bindir)/acsim-bin
exports += $(includedir)/ac_utils.H
clean += $(bindir)/acsim

$(bindir)/acsim-bin: $(acsimdir)/acsim.c $(acsimdir)/acsim.h $(includedir)/ac_decoder.h
	@echo "Building ArchC Simulator Generator"
	$(CC) -g -I$(includedir) -o $@ $< -L$(libdir) -lacpp
	rm -f $(bindir)/acsim
	$(LN) $(set_env) $(bindir)/acsim

$(includedir)/%: $(acsimdir)/%
	cp $^ $@

libs += $(libdir)/ac_module.o

exports += $(includedir)/ac_arch.H
exports += $(includedir)/ac_arch_ref.H
exports += $(includedir)/ac_arch_dec_if.H
exports += $(includedir)/ac_instr.H
exports += $(includedir)/ac_instr_info.H
exports += $(includedir)/ac_module.H
exports += $(includedir)/ac_sighandlers.H

$(libdir)/ac_module.o: $(coredir)/ac_module.cpp $(coredir)/ac_module.H
	@echo "Compiling ArchC Module Class"
	$(CXX) -g -I. -I$(SYSTEMC)/include -o $@ -c $<

$(includedir)/%.H: $(coredir)/%.H
	cp $^ $@

$(includedir)/%.h: $(coredir)/%.h
	cp $^ $@

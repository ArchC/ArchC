libs += $(libdir)/ac_tlm_port.o

exports += $(includedir)/ac_tlm_protocol.H
exports += $(includedir)/ac_tlm_port.H

$(libdir)/ac_tlm_port.o: $(tlmdir)/ac_tlm_port.cpp $(tlmdir)/ac_tlm_port.H
	@echo "Compiling ArchC TLM Port Module"
	$(CXX) -g -I. -I$(SYSTEMC)/include -I$(SYSTEMC)/include/sysc/tlm -I$(storagedir)/ -o $@ -c $<

$(includedir)/%.H: $(tlmdir)/%.H
	cp $^ $@

$(includedir)/%.h: $(tlmdir)/%.h
	cp $^ $@

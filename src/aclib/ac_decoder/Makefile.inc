libs += $(libdir)/ac_decoder_rt.o
libs += $(libdir)/ac_decoder.o
exports += $(includedir)/ac_decoder.h
exports += $(includedir)/ac_decoder_rt.H

$(libdir)/ac_decoder.o: $(decoderdir)/ac_decoder.c $(decoderdir)/ac_decoder.h
	@echo "Compiling ArchC Decoder Generator"
	@$(CC) -g -o $@ -c $<

$(libdir)/ac_decoder_rt.o: $(decoderdir)/ac_decoder_rt.cpp $(decoderdir)/ac_decoder_rt.H
	@echo "Compiling ArchC Decoder Generator (runtime)"
	@$(CXX) -g -I. -I$(SYSTEMC)/include -o $@ -c $<

$(includedir)/ac_decoder.h: $(decoderdir)/ac_decoder.h
	cp $^ $@

$(includedir)/ac_decoder_rt.H: $(decoderdir)/ac_decoder_rt.H
	cp $^ $@

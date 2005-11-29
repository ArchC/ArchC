exports += $(includedir)/ac_encoder.H

$(includedir)/%.H: $(encoderdir)/%.H
	cp $^ $@

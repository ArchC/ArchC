exports += $(includedir)/ac_debug_model.H
exports += $(includedir)/ac_log.H
exports += $(includedir)/ac_msgbuf.H
exports += $(includedir)/ac_sto_stats.H
exports += $(includedir)/ac_utils.H
exports += $(includedir)/archc.H
exports += $(includedir)/elf32-tiny.h

$(includedir)/%.H: $(utilsdir)/%.H
	cp $^ $@

$(includedir)/%.h: $(utilsdir)/%.h
	cp $^ $@

exports += $(includedir)/breakpoints.H
exports += $(includedir)/ac_gdb.H
exports += $(includedir)/ac_gdb_interface.H

$(includedir)/%.H: $(gdbdir)/%.H
	cp $^ $@

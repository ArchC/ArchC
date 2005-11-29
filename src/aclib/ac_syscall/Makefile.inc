exports += $(includedir)/ac_syscall.H
exports += $(includedir)/ac_syscall.def
exports += $(includedir)/ac_syscall_codes.h

$(includedir)/%: $(syscalldir)/%
	cp $^ $@

libs += $(libdir)/ac_syscall.o

exports += $(includedir)/ac_syscall.H
exports += $(includedir)/ac_syscall.def
exports += $(includedir)/ac_syscall_codes.h

$(libdir)/ac_syscall.o: $(syscalldir)/ac_syscall.cpp
	@echo "Compiling ArchC System Call Auxiliary Functions"
	$(CXX) -g -I. -o $@ -c $<

$(includedir)/%: $(syscalldir)/%
	cp $^ $@

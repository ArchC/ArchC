libs += $(libdir)/ac_storage.o

exports += $(includedir)/ac_cache.H
exports += $(includedir)/ac_cache_if.H
exports += $(includedir)/ac_inout_if.H
exports += $(includedir)/ac_mem.H
exports += $(includedir)/ac_memport.H
exports += $(includedir)/ac_ptr.H
exports += $(includedir)/ac_regbank.H
exports += $(includedir)/ac_reg.H
exports += $(includedir)/ac_storage.H

$(libdir)/ac_storage.o: $(storagedir)/ac_storage.cpp $(storagedir)/ac_storage.H $(storagedir)/ac_inout_if.H $(storagedir)/ac_ptr.H
	@echo "Compiling ArchC Storage Class"
	@$(CXX) -g -o $@ -c $<

$(includedir)/%.H: $(storagedir)/%.H
	cp $^ $@

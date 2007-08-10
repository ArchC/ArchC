/ALLDEPFILES = \\/ a\
	xxxxx-tdep.c \\

/abug-rom.o:/ i\
xxxxx-tdep.o: xxxxx-tdep.c $(defs_h) $(frame_h) $(frame_unwind_h) \\\
	$(frame_base_h) $(trad_frame_h) $(gdbcmd_h) $(gdbcore_h) \\\
	$(inferior_h) $(symfile_h) $(arch_utils_h) $(regcache_h) \\\
	$(gdb_string_h) $(dis_asm_h)

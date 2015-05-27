lib_LIBRARIES = libpowersc.a

AM_CPPFLAGS = -I. @SYSTEMC_CFLAGS@

include_HEADERS = psc_sampler.h psc_base_integer.h psc_objinfo.h psc_objinfo_if.h psc_obj_rep.h psc_obj_rep_base.h debug_power.h powersc.h psc_bit.h psc_bv.h psc_clock.h psc_int.h psc_logic.h psc_lv.h psc_sensitive.h psc_signal.h psc_signal_bool.h psc_uint.h psc_randgen.h psc_tables.h psc_misc.h psc_interp.h psc_techlib.h psc_macromodel.h config.h

libpowersc_a_CXXFLAGS = -Wall -Wno-deprecated -Wno-char-subscripts -std=c++11 -std=gnu++11

dist_EXTRA_libpowersc_a_SOURCES = psc_objinfo.cpp psc_bv.cpp psc_int.cpp psc_lv.cpp psc_uint.cpp  

libpowersc_a_SOURCES = powersc.cpp psc_sampler.cpp psc_obj_rep.cpp psc_bit.cpp psc_signal_bool.cpp psc_clock.cpp psc_logic.cpp psc_randgen.cpp psc_tables.cpp psc_misc.cpp psc_interp.cpp psc_techlib.cpp psc_macromodel.cpp



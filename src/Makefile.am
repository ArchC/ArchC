## Process this file with automake to produce Makefile.in


EXTRA_DIST = main.dox

if POWERSC_SUPPORT
   PW_VAR = powersc
else
   PW_VAR = 
endif

if SYSTEMC_SUPPORT
  SC_DEP_DIRS = $(PW_VAR) aclib acsim actsim accsim
else
  SC_DEP_DIRS =
endif

SUBDIRS = replace acpp $(SC_DEP_DIRS) acbinutils

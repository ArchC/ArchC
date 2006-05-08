## Process this file with automake to produce Makefile.in


EXTRA_DIST = main.dox


if SYSTEMC_SUPPORT
  SC_DEP_DIRS = aclib acsim actsim accsim
else
  SC_DEP_DIRS =
endif

SUBDIRS = replace acpp $(SC_DEP_DIRS) acbinutils

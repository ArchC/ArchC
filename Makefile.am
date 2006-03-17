## Process this file with automake to produce Makefile.in

## Automake options
AUTOMAKE_OPTIONS = foreign

## Subdirectories
SUBDIRS = src

## Special ArchC configuration file installation
sysconf_DATA = archc.conf
archc.conf: archc.conf.m4 Makefile
	m4 -P -Dsystemc_path=${SC_DIR} -Dcompiler=${CXX} -Dsystemc_target=${TARGET_ARCH} -Dbinutils_path=${BINUTILS_DIR} $< > $@

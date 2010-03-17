## Process this file with automake to produce Makefile.in

# use this flag at command line in order to check the distribution
# correctly - it is only here to remember that ;)
#DISTCHECK_CONFIGURE_FLAGS = "--with-systemc"

EXTRA_DIST = archc.conf.m4 \
             BUGS \
             acstone \
             doc

## Automake options
AUTOMAKE_OPTIONS = foreign
#ACLOCAL_AMFLAGS = -I m4
## Subdirectories
SUBDIRS = src

## Special ArchC configuration file installation
sysconf_DATA = archc.conf
archc.conf: archc.conf.m4 Makefile
	m4 -P -Dsystemc_path=${SC_DIR} -Dcompiler=${CXX} -Dsystemc_target=${TARGET_ARCH} -Dtlm_path=${TLM_DIR} -Dbinutils_path=${BINUTILS_DIR} -Dgdb_path=${GDB_DIR} $< > $@

dist-hook:
	rm -rf `find $(distdir) -name .svn`

uninstall-hook:
	rm archc.conf

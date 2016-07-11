## Process this file with automake to produce Makefile.in

EXTRA_DIST = archc.conf.m4 \
             BUGS \
             doc

## Automake options
AUTOMAKE_OPTIONS = foreign
#ACLOCAL_AMFLAGS = -I m4
## Subdirectories
SUBDIRS = src

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pc/archc.pc pc/powersc.pc

## Special ArchC configuration file installation
sysconf_DATA = archc.conf env.sh
archc.conf: archc.conf.m4 Makefile
	m4 -P -Dcompiler=${CXX} -Dbinutils_path=${BINUTILS_DIR} -Dgdb_path=${GDB_DIR} $< > $@

dist-hook:
	rm -rf `find $(distdir) -name .svn`

uninstall-hook:
	rm archc.conf

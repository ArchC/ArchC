#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([archc], [2.4.1])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER(config.h)
#AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([subdir-objects])

AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/acsim/acsim.c])
AC_CONFIG_LIBOBJ_DIR([src/replace])

AC_DISABLE_SHARED

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL

# pkg-config
PKG_PROG_PKG_CONFIG
if test "x${PKG_CONFIG}" = "x" ; then
   AC_MSG_ERROR([pkg-config tool not found. Install it or set PKG_CONFIG environment variable to that path tool. Exiting...])
fi

# Getting system configuration - copy from SystemC
AC_CANONICAL_HOST
TARGET_ARCH=
case "$host" in
  x86_64*linux*)
    TARGET_ARCH="linux64"
    ;;
  *linux*)
    TARGET_ARCH="linux"
    ;;
  *darwin*)
    TARGET_ARCH="macosx"
    ;;
esac

AC_ARG_WITH(systemc,
      AC_HELP_STRING([--with-systemc=PATH],
                     [Sets the directory where SystemC is installed.]),
[
	# Hard set of SystemC configuration ignoring pkg-config to legacy support
	AC_DEFINE([HAVE_CUSTOM_SYSTEMC], [1], [Custom SystemC])
	SYSTEMC="$withval"
	AM_CONDITIONAL(HAVE_SYSTEMC, true)
	AC_DEFINE([HAVE_SYSTEMC], [1], [Have SystemC])
	AM_CONDITIONAL(HAVE_TLM, true)
	AC_DEFINE([HAVE_TLM], [1], [Have TLM])
	AM_CONDITIONAL(HAVE_TLM2, true)
	AC_DEFINE([HAVE_TLM2], [1], [Have TLM2])
	SYSTEMC_CFLAGS="-pthread  -I$SYSTEMC/include"
	SYSTEMC_LIBS="-L$SYSTEMC/lib-$TARGET_ARCH -lsystemc"
	TLM_CFLAGS="-I$SYSTEMC/include"
	TLM_LIBS="-L$SYSTEMC/lib-$TARGET_ARCH -lsystemc"
	TLM2_CFLAGS="-I$SYSTEMC/include"
	TLM2_LIBS="-L$SYSTEMC/lib-$TARGET_ARCH -lsystemc"

	EXPORT_SYSTEMC_LIB="$SYSTEMC/lib-$TARGET_ARCH"
	AC_SUBST(EXPORT_SYSTEMC_LIB)
	EXPORT_SYSTEMC_PKG="$SYSTEMC/lib-$TARGET_ARCH/pkgconfig"
	AC_SUBST(EXPORT_SYSTEMC_PKG)
],
[
	PKG_CHECK_MODULES(SYSTEMC, [systemc >= 2.3.0], [have_systemc=yes], [have_systemc=no])
	AS_IF([test "x$have_systemc" = "xyes"],
	      [
		AC_DEFINE([HAVE_SYSTEMC], [1], [Have SystemC])
		for SC_FLAG in $SYSTEMC_LIBS; do
			[[[ $SC_FLAG == \-L* ]]] && EXPORT_SYSTEMC_LIB=${SC_FLAG:2} && break
		done
		AC_SUBST(EXPORT_SYSTEMC_LIB)
		EXPORT_SYSTEMC_PKG="$EXPORT_SYSTEMC_LIB/pkgconfig"
		AC_SUBST(EXPORT_SYSTEMC_PKG)
	      ])
	AM_CONDITIONAL(HAVE_SYSTEMC, test "x$have_systemc" = "xyes")

	PKG_CHECK_MODULES(TLM, [tlm >= 1], [have_tlm=yes], [have_tlm=no])
	AS_IF([test "x$have_tlm" = "xyes"],
	      [AC_DEFINE([HAVE_TLM], [1], [Have TLM])])
	AM_CONDITIONAL(HAVE_TLM, test "x$have_tlm" = "xyes")

	PKG_CHECK_MODULES(TLM2, [tlm >= 2], [have_tlm2=yes], [have_tlm2=no])
	AS_IF([test "x$have_tlm2" = "xyes"],
	      [AC_DEFINE([HAVE_TLM2], [1], [Have TLM2])])
	AM_CONDITIONAL(HAVE_TLM2, test "x$have_tlm2" = "xyes")
])

# SystemC base and TLM library options
BINUTILS_DIR=
GDB_DIR=

AC_ARG_WITH(binutils,
            AC_HELP_STRING([--with-binutils=PATH],
                           [Sets the directory where the Binutils source files are stored.]),
            [BINUTILS_DIR=$withval],
            [AC_MSG_NOTICE([Binutils path not specified.])])

AC_ARG_WITH(gdb,
            AC_HELP_STRING([--with-gdb=PATH],
                           [Sets the directory where the Gdb source files are stored.]),
            [GDB_DIR=$withval],
            [AC_MSG_NOTICE([Gdb path not specified.])])

AC_SUBST(BINUTILS_DIR)
AC_SUBST(GDB_DIR)

AC_ARG_ENABLE(hlt,
              AS_HELP_STRING([--disable-hlt], [Disable High Level Trace feature]),
              [disable_hlt=yes],[])

AS_IF([ test "$disable_hlt" != "yes" ],
      [ AC_CHECK_HEADER([elfutils/libdwfl.h],[],[AC_MSG_ERROR(["Please, install libdw-dev"])])], 
      [])


AM_CONDITIONAL([HLT_SUPPORT], [ ! test "$disable_hlt" = "yes" ])

#Uncomment this lines to use clang compiler or redefine CC and CXX in command line
#CC="clang-3.5 -std=c11"
#CXX="clang++-3.5 -std=c++11"

# Sets global flags
CFLAGS="$CFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -Wall"

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

#AC_CHECK_HEADER([elfutils/libdwfl.h],[],[AC_MSG_ERROR(["Please, install libdw-dev"])])]

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 memset socket strchr strdup strerror strstr strtol])

AC_CHECK_FUNC([getopt_long_only],,
[
  AC_LIBOBJ(getopt)
  AC_LIBOBJ(getopt1)
])

# Configuration files
AC_CONFIG_FILES(
[
  Makefile 
  Doxyfile 
  src/Makefile 
  src/replace/Makefile 
  src/acpp/Makefile 
  src/aclib/Makefile 
  src/aclib/ac_core/Makefile 
  src/aclib/ac_decoder/Makefile 
  src/aclib/ac_gdb/Makefile 
  src/aclib/ac_rtld/Makefile
  src/aclib/ac_storage/Makefile 
  src/aclib/ac_cache/Makefile 
  src/aclib/ac_stats/Makefile
  src/aclib/ac_syscall/Makefile
  src/aclib/ac_tlm/Makefile
  src/aclib/ac_utils/Makefile
  src/acsim/Makefile
  src/actsim/Makefile
  src/accsim/Makefile
  src/acbinutils/Makefile
  src/acbinutils/binutils/gas/config/tc-xxxxx.c
  src/powersc/Makefile
  pc/archc.pc
  pc/powersc.pc
  env.sh
])

# Output
AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        ======

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        bindir:                 ${bindir}
        SystemC:                ${SYSTEMC_LIBS}
        TLM:                    ${TLM_LIBS}
        TLM2:                   ${TLM2_LIBS}
        binutilsdir:            ${BINUTILS_DIR}
        gdbdir:                 ${GDB_DIR}

        cc:                     ${CC}
        cxx:                    ${CXX}
        cflags:                 ${CFLAGS}
        cxxflags:               ${CXXFLAGS}
        ldflags:                ${LDFLAGS}
])

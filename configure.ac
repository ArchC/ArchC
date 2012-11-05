#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([archc], [2.2])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER(config.h)
#AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CONFIG_SRCDIR([src/acsim/acsim.c])
AC_CONFIG_LIBOBJ_DIR([src/replace])

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
AC_SUBST(TARGET_ARCH)

AC_DISABLE_SHARED

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL

# SystemC base and TLM library options
SC_DIR=
TLM_DIR=
BINUTILS_DIR=
GDB_DIR=

AC_ARG_WITH(systemc,
            AC_HELP_STRING([--with-systemc=PATH],
                           [Sets the directory where SystemC is installed.]),
            [SC_DIR=$withval],
            [AC_MSG_NOTICE([ArchC will be compiled without SystemC.])])

AC_ARG_WITH(tlm,
            AC_HELP_STRING([--with-tlm=PATH],
                           [Sets the directory where the TLM libraries are installed.]),
            [TLM_DIR=$withval],
            [AC_MSG_NOTICE([ArchC will be compiled without TLM.])])

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

AC_SUBST(SC_DIR)
AC_SUBST(TLM_DIR)
AC_SUBST(BINUTILS_DIR)
AC_SUBST(GDB_DIR)

AM_CONDITIONAL([TLM_SUPPORT], [ ! test -z "$TLM_DIR"])
AM_CONDITIONAL([SYSTEMC_SUPPORT], [ ! test -z "$SC_DIR" ])

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

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
  src/aclib/ac_stats/Makefile
  src/aclib/ac_syscall/Makefile
  src/aclib/ac_tlm/Makefile
  src/aclib/ac_utils/Makefile
  src/acsim/Makefile
  src/actsim/Makefile
  src/accsim/Makefile
  src/acbinutils/Makefile
  src/acbinutils/binutils/gas/config/tc-xxxxx.c
])

# Output
AC_OUTPUT

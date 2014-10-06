#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([archc], [2.2])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER(config.h)
#AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

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
AC_PROG_CC_C99
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

AC_ARG_WITH(powersc,
            AC_HELP_STRING([--with-powersc=PATH],
                           [Sets the directory where the the SystemC source code are stored.]),
            [PW_DIR=$withval],
            [AC_MSG_NOTICE([ArchC will be compiled without PowerSC.])])

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
AC_SUBST(PW_DIR)

AM_CONDITIONAL([TLM_SUPPORT], [ ! test -z "$TLM_DIR"])
AM_CONDITIONAL([SYSTEMC_SUPPORT], [ ! test -z "$SC_DIR" ])
AM_CONDITIONAL([POWERSC_SUPPORT], [ ! test -z "$PW_DIR" ])


# set the SystemC headers directory
SCLIB_DIR="$SC_DIR"
SCLIB_INCLUDES="-I$SCLIB_DIR/include"
SCLIB_SRC_INCLUDES="-I$PW_DIR/src"
#SCLIB_SRC_INCLUDES="-I$SCLIB_DIR/src"
AC_SUBST(SCLIB_DIR)
AC_SUBST(SCLIB_INCLUDES)
AC_SUBST(SCLIB_SRC_INCLUDES)


SCLIB_SRC_DIR="$PW_DIR/sysc"
SC_DT_BIT_DIR="$SCLIB_SRC_DIR/datatypes/bit"
SC_COMM_DIR="$SCLIB_SRC_DIR/communication"
SC_KERNEL_DIR="$SCLIB_SRC_DIR/kernel"
#AC_CHECK_FILES($SC_DT_BIT_DIR/sc_bit.cpp \
#          $SC_DT_BIT_DIR/sc_logic.cpp \
#          $SC_COMM_DIR/sc_clock.cpp \
#          $SC_KERNEL_DIR/sc_wait.cpp \
#          $SC_KERNEL_DIR/sc_join.cpp ,
#          ,
#          AC_MSG_ERROR([SystemC source file not found!!!. See config.log for details.])
#          )

AC_SUBST(SC_DT_BIT_DIR) 
AC_SUBST(SC_COMM_DIR)
AC_SUBST(SC_KERNEL_DIR)


# set the SystemC library directory
SCLIB_LIBDIR="$SCLIB_DIR/lib-linux"
LDFLAGS="-L$SCLIB_LIBDIR $LDFLAGS"

# Sets global flags
CFLAGS="$CFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -Wall"

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
  src/powersc/Makefile
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
        systemcdir:		${SC_DIR}
        tlmdir:                 ${TLM_DIR}
        binutilsdir:            ${BINUTILS_DIR}
        gdbdir:                 ${GDB_DIR}

        cc:                     ${CC}
        cxx:                    ${CXX}
        cflags:                 ${CFLAGS}
        cxxflags:               ${CXXFLAGS}
        ldflags:                ${LDFLAGS}
])

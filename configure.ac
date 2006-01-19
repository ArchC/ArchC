#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ArchC, 2.0alpha1)
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/accsim/accsim.c])
AC_CONFIG_AUX_DIR(config)
#AM_CONFIG_HEADER([archc_config.h])

# Custom macro definitions


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_LEX
AC_PROG_YACC

# SystemC base and TLM library options
SC_DIR=
TLM_DIR=
MAYBE_TLM=
MAYBE_TLM_OBJ=

AC_ARG_WITH(systemc,
            AC_HELP_STRING([--with-systemc=PATH],
                           [Sets the directory where SystemC is installed.]),
            [SC_DIR=$withval],
            [AC_MSG_ERROR([Setting SystemC path via --with-systemc=PATH is required.])])

AC_ARG_WITH(tlm,
            AC_HELP_STRING([--with-tlm=PATH],
                           [Sets the directory where the TLM libraries are installed]),
            [TLM_DIR=$withval; MAYBE_TLM=ac_tlm; MAYBE_TLM_OBJ=ac_tlm/ac_tlm_port.o],
            [AC_MSG_NOTICE([ArchC will be compiled without TLM.])])

AC_SUBST(SC_DIR)
AC_SUBST(TLM_DIR)
AC_SUBST(MAYBE_TLM)
AC_SUBST(MAYBE_TLM_OBJ)

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

# Configuration files
AC_CONFIG_FILES([Makefile src/Makefile src/acpp/Makefile src/aclib/Makefile src/aclib/ac_core/Makefile src/aclib/ac_decoder/Makefile src/aclib/ac_encoder/Makefile src/aclib/ac_gdb/Makefile src/aclib/ac_storage/Makefile src/aclib/ac_syscall/Makefile src/aclib/ac_tlm/Makefile src/aclib/ac_utils/Makefile src/acsim/Makefile])

# Output
AC_OUTPUT

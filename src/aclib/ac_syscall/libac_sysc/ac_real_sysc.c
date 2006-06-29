/**
 * @file      ac_real_sysc.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
 *
 * @brief     
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* some syscalls that trap to simulator functions */

#define AC_SYSC(NAME,LOCATION) \
int NAME() { \
  goto *LOCATION; \
}

#include "ac_syscall.def"

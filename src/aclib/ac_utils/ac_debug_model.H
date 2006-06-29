/**
 * @file      ac_debug_model.H
 * @author    Marcus Bartholomeu
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     The ArchC model debug library.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifdef DEBUG_MODEL
#  include <stdio.h>
#  define dbg_printf(args...) fprintf (stderr, "DBG: " args)
#  define _d_(arg) arg
#else
#  define dbg_printf(args...)
#  define _d_(arg)
#endif

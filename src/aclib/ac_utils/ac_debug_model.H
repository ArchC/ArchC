/********************************************************/
/* The ArchC model debug library.                       */
/* Author: Marcus Bartholomeu                           */
/*                                                      */
/* For more information on ArchC, please visit:         */
/* http://www.archc.org                                 */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

#ifdef DEBUG_MODEL
#  include <stdio.h>
#  define dbg_printf(args...) fprintf (stderr, "DBG: " args)
#  define _d_(arg) arg
#else
#  define dbg_printf(args...)
#  define _d_(arg)
#endif

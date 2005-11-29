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
#  define dprintf(...) fprintf (stderr, "DBG: " __VA_ARGS__)
#  define _d_(arg) arg
#else
#  define dprintf(...)
#  define _d_(arg)
#endif

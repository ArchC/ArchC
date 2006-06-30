/**
 * @file      ac_cache_if.cpp
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include "ac_cache_if.H"

void bind(ac_cache_if& previous, ac_cache_if& next) {
  previous.bindToNext(next);
  next.bindToPrevious(previous);
}

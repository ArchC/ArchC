/**
 * @file      ac_cache_if.H
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

#ifndef _AC_CACHE_IF_H
#define _AC_CACHE_IF_H

#include "ac_parms.H"

class ac_cache_if {
public:
  virtual void request_block(ac_cache_if* client, unsigned address, unsigned size_bytes) = 0;
  virtual void request_write_byte(ac_cache_if* client, unsigned address, unsigned char datum) = 0;
  virtual void request_write_half(ac_cache_if* client, unsigned address, unsigned short datum) = 0;
  virtual void request_write(ac_cache_if* client, unsigned address, ac_word datum) = 0;
  virtual void request_write_block(ac_cache_if* client, unsigned address, char* datum, unsigned size_bytes) = 0;

  virtual void response_block(char* block) = 0;
  virtual void response_write_byte() = 0;
  virtual void response_write_half() = 0;
  virtual void response_write() = 0;
  virtual void response_write_block() = 0;

  virtual void bindToNext(ac_cache_if& next) = 0;
  virtual void bindToPrevious(ac_cache_if& previous) = 0;
  

};

void bind(ac_cache_if& previous, ac_cache_if& next);

#endif

/**
 * @file      ac_mem.H
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

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include "archc.H"
#include "ac_parms.H"
#include "ac_storage.H"
#include "ac_cache_if.H"

#include <string>
#include "fstream"

#ifndef _AC_MEM_H
#define _AC_MEM_H

#define W_WORD 4
#define W_HALF 2
#define W_BYTE 1



class ac_mem: public ac_storage , public ac_cache_if{

  ac_cache_if* next_level;       //!Next lower level of the hierarchy
  ac_cache_if* previous_level;   //!Previous upper level of the hierarchy


private:
       char *request_buffer;
       int write_size;
       bool read_access_type;
       char* datum_ref;
       ac_cache_if* client_global;
       bool request_block_event;
       bool request_write_block_event;
       bool request_write_event;


      unsigned char replace_status;
      unsigned requested_address;


public:
#ifdef AC_TRACE
  ofstream trace;
#endif
//  FILE* trace;
  //!Reading a word
  virtual ac_word ac_mem::read( unsigned address );

  //!Reading a byte
  virtual unsigned char ac_mem::read_byte( unsigned address );

  //!Reading half word
  virtual ac_Hword ac_mem::read_half( unsigned address );

  //!Writing a word
  virtual void ac_mem::write( unsigned address, ac_word datum );

  //!Writing a byte
  virtual void ac_mem::write_byte( unsigned address, unsigned char datum );

  //!Writing half word
  virtual void ac_mem::write_half( unsigned address, unsigned short datum );

  //!Class's Constructor.
  ac_mem::ac_mem( char *n, unsigned s);

  //!Class's Destructor
  ac_mem::~ac_mem();

  //
//  void ac_mem::bindTo(ac_mem& lower);

  void ac_mem::stall();

  void ac_mem::ready();


  void request_block(ac_cache_if* client, unsigned address, unsigned size_bytes);
  void request_write_byte(ac_cache_if* client, unsigned address, unsigned char datum);
  void request_write_half(ac_cache_if* client, unsigned address, unsigned short datum);
  void request_write(ac_cache_if* client, unsigned address, ac_word datum);
  void request_write_block(ac_cache_if* client, unsigned address, char* datum, unsigned size_bytes);

  void response_block(char* block);
  void response_write_byte();
  void response_write_half();
  void response_write();
  void response_write_block();

  void bindToNext(ac_cache_if& next);
  void bindToPrevious(ac_cache_if& previous);

  void process_request();

};
#endif


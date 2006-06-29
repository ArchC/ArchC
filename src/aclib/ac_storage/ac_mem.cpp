/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_mem.cpp
 * @author    Pablo Viana <pvs@cin.ufpe.br>
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            CIn - UFPE
 *
 * @version   1.0
 * @date      Thu Sep 22 11:47:36 2005
 *
 * @brief     The ArchC Mem device class. Implementation file.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */
 
//////////////////////////////////////////////////////////


#include "ac_mem.H"
#include "ac_resources.H"

//!Read a word from the address passed as parameter
  ac_word ac_mem::read( unsigned address ) /*const*/
  {
      return (ac_storage::read(address));
  }

  //!Reading a byte
  unsigned char ac_mem::read_byte( unsigned address )
  {
      return (ac_storage::read_byte(address));
  }

  //!Reading half word
  ac_Hword ac_mem::read_half( unsigned address )
  {
      return (ac_storage::read_half(address));
  }


  //!Writing into the mem
  void ac_mem::write( unsigned address, ac_word datum )
  {
       this->ac_storage::write(address, datum);
  }


  //!Writing a byte to an address.
  void ac_mem::write_byte( unsigned address, unsigned char datum )
  {
       this->ac_storage::write_byte(address, datum);
  }


  //!Writing a halfword to an address.
  void ac_mem::write_half( unsigned address, unsigned short datum )
  {
      this->ac_storage::write_half(address, datum);
  }


  ac_mem::ac_mem( char *n, unsigned s) :
    ac_storage(n, s)
  {
    request_block_event = false;
    request_write_block_event = false;
    request_write_event = false;
   	next_level = NULL;
   	previous_level = NULL;
//    request_buffer = new char[block_size*(AC_WORDSIZE/8)];
    datum_ref = new char[4];
  }




  //!Destructor
  ac_mem::~ac_mem()
  {
//      delete[] request_buffer;
      delete[] datum_ref;
  }

//  void ac_mem::bindTo(ac_mem& lower){  //were ac_mem
//       this->next_level = &lower;
//       lower.previous_level = this;
//  }


/*
################################################################################
##############           PROCESSOR STALLING              #######################
################################################################################
*/
  void ac_mem::stall(){
      ac_resources::ac_wait();
//       cout << "Wait "<< this->get_name() << endl;      
  }


  void ac_mem::ready(){
      ac_resources::ac_release();
//      this->replace_status = 0;
//       cout << "Ready "<< this->get_name() << endl;
  }


/*
################################################################################
##############                 INTERFACE                 #######################
################################################################################
*/

  void ac_mem::request_block(ac_cache_if* client, unsigned address, unsigned size_bytes)
  {
//  	    cout << "requesting from" << this->get_name() << endl;
//  	    cout << "size in bytes: " << size_bytes << endl;
       client_global = client;
       request_buffer = new char[size_bytes*(AC_WORDSIZE/8)];
//       cout << "request block em MEM no address: " << address << endl;
       for (unsigned offset_word = 0; offset_word < size_bytes; offset_word+=AC_WORDSIZE/8)
       {
          *(ac_word *)(request_buffer + offset_word) = this->read(address + offset_word);
//           cout << " bloco requisitado" << *(ac_word *)(request_buffer + offset_word) << " from: " << (address + offset_word) << endl;
       }
       request_block_event = true;
       //client->response_block(request_buffer);

   }

  void ac_mem::process_request() {
       if (request_block_event) {
          request_block_event = false;
          client_global->response_block(request_buffer);
       }else if (request_write_block_event) {
          request_write_block_event = false;
//          cout << "respondendo write_block " << this->name << endl;
          client_global->response_write_block();
       }else if (request_write_event) {
          request_write_event = false;
          client_global->response_write();
       }
       //while (true)
       //{
       //	     wait(request_block_event);
       //      client_global->response_block(request_buffer);
       //}
  }

  void ac_mem::request_write_byte(ac_cache_if* client, unsigned address, unsigned char datum)
  {
  	   client_global = client;
       ac_mem::write_byte(address, datum);
       request_write_event = true;
       //notify(SC_ZERO_TIME, request_write_event);
       //client->response_write();
  }

  void ac_mem::request_write_half(ac_cache_if* client, unsigned address, unsigned short datum)
  {
  	   client_global = client;
       ac_mem::write_half(address, datum);
       request_write_event = true;
       //notify(SC_ZERO_TIME, request_write_event);
       //client->response_write();
  }

  void ac_mem::request_write(ac_cache_if* client, unsigned address, ac_word datum)
  {
  	   client_global = client;
       ac_mem::write(address, datum);
       request_write_event = true;
//       cout << " mem escrita Write: " << read(address) << endl;
       //client->response_write();
  }

  void ac_mem::request_write_block(ac_cache_if* client, unsigned address, char* datum, unsigned size_bytes)
  {

  	   client_global = client;
  	   for (unsigned offset_word = 0; offset_word < size_bytes; offset_word+=AC_WORDSIZE/8)
//  	   for (unsigned offset_word = 0; offset_word < size; offset_word++)
  	   {
           write(address + offset_word/**AC_WORDSIZE/8*/, *(ac_word*)(datum + offset_word/**AC_WORDSIZE/8*/));
//           cout << "buffer recebido em " << (int)datum << endl;
//           cout << " mem escrita Block: " << read(address + offset_word) << endl;
//           cout << "Data stored: " << hex << read(address + offset_word) << " Address: " << (address + offset_word) << endl;
       }
//       cout << "Guardei o bloco no proximo nivel" << endl;
       request_write_block_event = true;
       //client->response_write_block();

  }

  void ac_mem::response_block(char* block)
  {
/*       *(slot_tag) = address_tag;
       *(slot_valid) = true;
       *(slot_dirty) = false;
       for (unsigned offset_word = 0; offset_word < block_size; offset_word++)
       {
           *(ac_word *)(slot_data + offset_word*AC_WORDSIZE/8) = *(ac_word *)(block + offset_word*(AC_WORDSIZE/8));
       }
       replace_status++;
       if(read_access_type)
            this->replaceBlockRead(requested_address);
       else
            this->replaceBlockWrite();
*/
       delete[] block;

  }


  void ac_mem::response_write_byte()
  {
//       this->ready(requested_address);
  }

  void ac_mem::response_write_half()
  {
//       this->ready(requested_address);
  }

  void ac_mem::response_write()
  {
       this->ready();
  }

  void ac_mem::response_write_block()
  {
/*  	replace_status++;
       if(read_access_type)
            this->replaceBlockRead(requested_address);
       else
            this->replaceBlockWrite();
*/
  }





  void ac_mem::bindToNext(ac_cache_if& next)
  {
       this->next_level = &next;
  }

  void ac_mem::bindToPrevious(ac_cache_if& previous)
  {
//       this->previous_level = (ac_mem&)&previous;
  }




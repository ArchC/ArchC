/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_cache.H
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
 * @brief     The ArchC cache simulation class.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include "ac_storage.H"
#include "ac_cache_if.H"

#include <string>
#include "fstream"



#ifndef _AC_CACHE_H
#define _AC_CACHE_H


//#ifndef AC_DETAIL
//#define AC_DETAIL
//#endif


#define WRITE_THROUGH true
//#define AC_TRACE true
//#define AC_DETAIL true

#ifndef RAND_MAX
  #define RAND_MAX 1024;
#endif

#define RANDOM 0
#define LRU 1
#define DEFAULT 2

#define W_WORD 4
#define W_HALF 2
#define W_BYTE 1


/////////////////////////////////////////////////////
//!
//!ArchC class for cache devices
//!This class contains  cache parameters for
//!behavioral simulation.
//!The user must define three parameters:
//!using macros for
//!BLOCK_SIZE, NUM_BLOCKS and POLICIES
//!
/////////////////////////////////////////////////////



class ac_cache: /*public sc_module,*/ public ac_storage , public ac_cache_if{

  unsigned block_size;           //!Number of data words in a block
  unsigned num_blocks;           //!Cache's number of lines (blocks)
  unsigned set_size;
  unsigned num_sets;
  unsigned * tag;                //!Tag that certifies the address correctness
  bool * valid;                  //!Bit-valid flag
  bool * dirty;
  unsigned codeSize;
  unsigned *  chosen;             //! Element elected to be replaced (SUBSTITUIR DEPOIS POR UNSIGNED)
  unsigned * tracker;            //! Array for tracking the cache accesses for the LRU strategy
  int strategy;
//  ac_storage* next_level;       //!Next lower level of the hierarchy
//  ac_storage* previous_level;   //!Previous upper level of the hierarchy
  ac_cache_if* next_level;       //!Next lower level of the hierarchy
  ac_cache_if* previous_level;   //!Previous upper level of the hierarchy


private:

      //!Define the position information
      unsigned offset;
      unsigned set;
      unsigned address_tag;

      //!Point to the correspondent memory address
      char *slot_data;
      unsigned *slot_tag;
      bool *slot_valid;
      bool *slot_dirty;
      unsigned element; //set' s line used

      unsigned char write_policy; //byte register with the write setup |7|6|5|4|3|2|1|0|
      // bit 0 - if set is write-through      // bit 4 - if set is write-allocate
      // bit 1 - if set is write-back         // bit 5 - if set is write-around
      // bit 2 - not used                     // bit 6 - not used
      // bit 3- not used                      // bit 7 - not used

       char *request_buffer;
       int write_size;
       bool read_access_type;
       char* datum_ref;
       ac_cache_if* client_global;
       bool request_block_event;
       bool request_write_block_event;
       bool request_write_event;




#ifdef  AC_TRACE  //! Trace files generation
      char padding;
      char access_type;
      short size_ref;
#endif

      int hit;       // reset the hit event

      unsigned char replace_status;
      unsigned requested_address;

      void ac_cache::addressing(unsigned address);               //slicing the address field
      void ac_cache::tracing(unsigned address, unsigned type);  //access trace file writing
//      void ac_cache::requestFromNext(unsigned address);         //request data from next storage level

      void ac_cache::replaceBlockWrite();            //replace the block with the required data
      void ac_cache::replaceBlockRead(unsigned address);            //replace the block with the required data

      void ac_cache::writingBack();              //write back to the lower level the modified blocks

      bool ac_cache::isWriteThrough();        //Checks the write-through policy
      bool ac_cache::isWriteBack();           //Checks the write-back policy
      bool ac_cache::isWriteAllocate();       //Checks the write-allocate policy
      bool ac_cache::isWriteAround();         //Checks the write-around policy


public:
#ifdef AC_TRACE
  ofstream trace;
#endif
//  FILE* trace;
  //!Reading the content of an address.
  //
  //!
  //! Takes as argument the 32-bit address. Searches inside the cache cobject the address asked.
  //!The operation can result in a Hit, when the addressed tag is present and the valid bit is ok, returning the resquested data
  //! else it can gets a Miss, asking the next hierarchical level for the data, through the "next_level" pointer.
  //!
  virtual ac_word ac_cache::read( unsigned address );

  //!Writing to an address.
  //
  //!
  //! Takes the address and the data to be writen. Searches inside the cache cobject the address asked.
  //!The operation can result in a Hit, when the addressed tag is present and thus the new data can be writen
  //! else it can gets a Miss, wrinting at the next_level too.
  //!


  //!Reading a byte
  virtual unsigned char ac_cache::read_byte( unsigned address );


  //!Reading half word
  virtual ac_Hword ac_cache::read_half( unsigned address );



  virtual void ac_cache::write( unsigned address, ac_word datum );

  //!Class's Constructor.
  //
  //!
  //!The needed arguments are 'n', 'bs', 'nb', 'ss' and 'st':
  //!'n' is the object's name, for instance: "IC2" (Instruction Cache Level 2).
  //!
  virtual void ac_cache::write_byte( unsigned address, unsigned char datum );

  //!Class's Constructor.
  //
  //!
  //!The needed arguments are 'n', 'bs', 'nb', 'ss' and 'st':
  //!'n' is the object's name, for instance: "IC2" (Instruction Cache Level 2).
  //!'st' is the Strategy used for replacement (LRU or RANDOM)
  //!

  virtual void ac_cache::write_half( unsigned address, unsigned short datum );

  //!Class's Constructor.
  //
  //!
  //!The needed arguments are 'n', 'bs', 'nb', 'ss' and 'st':
  //!'n' is the object's name, for instance: "IC2" (Instruction Cache Level 2).
  //!'bs' is the Block Size described by the number of words at each cache line.
  //!'nb' is the Number of Blocks (or lines) of the cache.
  //!'ss' is the Set Size (associativity) in number of grouped blocks at each set.
  //!'st' is the Strategy used for replacement (LRU or RANDOM)
  //!'wp' is the write policiy
  //!
  ac_cache::ac_cache( char *n, unsigned bs, unsigned nb, unsigned ss, unsigned st, unsigned char wp = 0x11);



  //!Class's Destructor
  ac_cache::~ac_cache();

  //
  void ac_cache::bindTo(ac_cache& lower);

 //!Method to initialize Data array dynamically
  void ac_cache::set_codeSize( unsigned s);

/*  void ac_cache::initialize(){
    ac_storage::initialize((unsigned)block_size*num_blocks);
    chosen = new char[this->num_sets];
    tracker = new char[this->set_size * this->num_sets];
    tag = new char[num_blocks*AC_WORDSIZE/8];
    valid = new bool[num_blocks];
    *valid = false;
    codeSize = 0;
  }
*/


  unsigned ac_cache::get_chosen(unsigned s);

  //!Method that implements the policies of replacement, based on the last access
  void ac_cache::update(unsigned s, unsigned e);

  unsigned ac_cache::get_codeSize();

  void ac_cache::stall();

//  void ac_cache::ready();
  void ac_cache::ready();


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

//       SC_HAS_PROCESS(ac_cache);
  void process_request();

};
#endif


/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC Storage Library for the ArchC architecture simulators
    Copyright (C) 2002-2004  The ArchC Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

/********************************************************/
/* The ArchC storage device base class.                 */
/* Author:  Sandro Rigo                                 */
/*                                                      */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

//////////////////////////////////////////////////////////
/*!\file ac_regbank.H
  \brief The ArchC storage device base class.

  This class contains  basic members and methods needed by
  all storage devices for behavioral simulation. 
*/
//////////////////////////////////////////////////////////
#ifndef _AC_REGBANK_H
#define _AC_REGBANK_H

#include <ios>
#include <sstream>
#include <string>
#include <fstream>
#include <iterator>

#include <systemc.h>

#include "ac_log.H"
  
using std::string;
using std::istringstream;
using std::fstream;
using std::ifstream;
using std::ios;

//////////////////////////////////////
//! ArchC class specialized in modeling register bank.
template<int nregs, class ac_word, class ac_Dword> class ac_regbank {
protected:
  typedef change_log<ac_word> chg_log;
  typedef list<chg_log> log_list;
#ifdef AC_UPDATE_LOG
  log_list changes;                 //!< Update log.
  fstream update_file;              //!< Update log binary file.
#endif
  
#ifdef AC_DELAY
  log_list delays;                  //!< Delayed update queue.
#endif

  double& time_step;

public:
  
  string Name;
//   ac_word* Data;
  ac_word Data[nregs];

  //!Reading the content of an address. Overloaded Method.
  ac_word read( unsigned address ){

    return Data[address];
  }

  //!Reading the content of an address. Overloaded Method.
  ac_Dword read_double( unsigned address ){

    return *((ac_Dword *)(Data+((address)*sizeof(ac_Dword)))); 
  }

  void write_double( unsigned address , ac_Dword datum  ){
#ifdef AC_UPDATE_LOG
    changes.push_back( chg_log(address, datum , sc_simulation_time()));
#endif
  
    *((ac_Dword *)(Data+((address)*sizeof(ac_Dword)))) = datum;
  
  }

  //!Writing to a register. Overloaded Method.
  void write( unsigned address , ac_word datum  ){
#ifdef AC_MEM_HIERARCHY
    if(ac_wait_sig)
      return;
#endif
  
#ifdef AC_UPDATE_LOG
    changes.push_back( chg_log(address, datum , sc_simulation_time()));
#endif
  
    //*((ac_word *)(Data+((address)*sizeof(ac_word)))) = datum;
    Data[address] = datum;
  
  }


#ifdef AC_DELAY
  //!Writing to a register. Overloaded Method.
  void write(unsigned address , ac_word datum,
             unsigned time) {
    delays.push_back( chg_log( address, datum, (time * time_step) + time_step + sc_simulation_time()));
  }
#endif

  //!Constructor
//   ac_regbank(string nm, unsigned s=sizeof(ac_word)):
//     Name(nm), Data(new ac_word[s]) {}

  ac_regbank(string nm, double& ts) : 
    time_step(ts), Name(nm) {}

  //!Dump the entire contents of a regbank device
  void dump(){
    fstream dfile;              //!< Dump file.
    unsigned i;

    char filename[Name.size() + 6];

    sprintf(filename, "%s.dump",Name.c_str());
    dfile.open(filename, ios::out);

    for( i=0; i<nregs; i++){
      dfile << dec << i << hex << "  " << read(i) <<endl;
    }
    dfile.close();
  }

  //!Method to load device content from a file.
  void load( char* file ){

    ifstream input;
    string read;
    string word;
    ac_word data;
    istringstream line;

    bool is_addr;

    int  addr=0;

    // Looking for initialization file.
    input.open(file);
    if(!input){
      AC_ERROR("Could not open input file:" << file);
    }
    else{
      
      while( !input.eof() ){

        line.clear();
        getline(input, read);
        line.str(read);
                        
        is_addr = 1;

        //Processing line
        while(line >> word){

          if( word == ".text" ){
            AC_ERROR("Should not load code into the register bank: " << Name);
            return;
          }
                                                
          if( word == ".data" ){
            continue;
          }

          //Processing word
          if( is_addr ){
            addr = strtol(word.c_str(), NULL, 16);
            is_addr = 0;
          }
          else{
            data = (ac_word)strtol( word.c_str(), NULL, 16);
            write(addr,data);
            addr+= sizeof(ac_word);
          }
        }
      }
    }
  }

#ifdef AC_DELAY

//!Method to commit delayed updates.
  void commit_delays( double time ){
    typename log_list::iterator itor;
    
    itor = delays.begin();
    
    //Sometimes, when a memory hierarchy is present and the processor spends some
    //cycles in a wait status, we may have to commit changes for every cycle <=
    //current time.
    while( delays.size() && (itor->time <= time) ){
      write( itor->addr, itor->value );
      itor = delays.erase( itor);
    }
  }
#endif

  // Destructor
  ~ac_regbank() {
//     delete[] Data;
  }

};

#endif //_AC_REGBANK_H

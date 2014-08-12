/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_regbank.H
 * @author    Sandro Rigo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Tue Sep 13 17:16:51 2005
 *
 * @brief     The ArchC storage device base class.
 *            This class contains  basic members and methods needed by
 *            all storage devices for behavioral simulation. 
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef _AC_REGBANK_H
#define _AC_REGBANK_H

#include <ios>
#include <sstream>
#include <string>
#include <fstream>
#include <iterator>

#include <systemc.h>

#include "ac_utils.H"
#include "ac_log.H"
#include "ac_utils.H"

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
  double& time_step;
#endif // AC_DELAY

public:
  
  string Name;
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
/*#ifdef AC_MEM_HIERARCHY
    if(ac_wait_sig)
      return;
#endif*/
  
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

  /// Default Constructor
  ac_regbank(string nm):
    Name(nm) {}

#ifdef AC_DELAY
  /// Constructor with delay parameter.
  ac_regbank(string nm, double& ts) : 
    time_step(ts), Name(nm) {}
#endif // AC_DELAY

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

  /**
   * Individual register access operator.
   * @param reg Index of the register in the bank.
   * @return Reference to register data element.
   */
  ac_word& operator [] (int reg) {
    return Data[reg];
  }

  /// Destructor
  ~ac_regbank() {
  }

};

#endif //_AC_REGBANK_H

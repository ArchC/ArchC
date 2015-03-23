/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      archc.H
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
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     Include file for ArchC library
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _ARCHC_H
#define _ARCHC_H

#ifndef AC_COMPSIM
#include <systemc.h>
#endif

#include <list>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "ac_parms.H"
#include "ac_log.H"
 
using std::list;
using std::setw;

extern int ac_stop_flag;
extern int ac_exit_status;
extern unsigned int ac_heap_ptr;

extern int ac_argc;                       
extern char **ac_argv;

// Prototypes
void ac_init_opt( int ac, char* av[]);
void ac_init_app( int ac, char* av[]);



//////////////////////////////////////////
// ArchC Defines                        //
//////////////////////////////////////////

#define ac_err stderr
#define IDENT         0


#define AC_ERROR( msg )    std::cerr<< "ArchC ERROR: " << msg  <<'\n'
#define AC_WARN( msg )     std::cerr<< "ArchC Warning: " << msg  <<'\n'
#define AC_SAY( msg )      std::cerr<< "ArchC: " << msg  <<'\n'

#define AC_RUN_ERROR( str, args...) \
  fprintf(stderr, "ArchC Runtime error (ac_pc=%#x; ac_instr_counter=%llu): " str, (int) ac_pc, ac_instr_counter , ##args)
#define AC_RUN_MSG( str )  fprintf(stderr, str);

#define ac_trace( f )      extern ofstream trace_file; extern bool ac_do_trace; ac_do_trace = 1; trace_file.open( f )
#define ac_close_trace     trace_file.close


#ifndef AC_COMPSIM

#ifndef AC_VERIFY

//ArchC simulator initialization
#define ac_init( processor) \
  extern char* appfilename; \
  ac_init_opt( ac, av);\
  ac_init_app( ac, av);\
  extern ac_storage* IMem; \
  IMem = processor.IM; \
  processor.APP_MEM->load(appfilename); \
  processor.time_step =  clk_cycle; \
  processor.set_args(ac_argc, ac_argv);\
  ac_initialize();\

#else

//ArchC simulator initialization
#define ac_init( processor) \
  extern char* appfilename; \
  ac_init_opt( ac, av);\
  ac_init_app( ac, av);\
  extern ac_storage* IMem; \
  IMem = processor.IM; \
  processor.APP_MEM->load(appfilename); \
  processor.time_step =  clk_cycle; \
  processor.set_args(ac_argc, ac_argv);\
  processor.set_queue(av[0]);\
  ac_initialize();\

#endif //ndef AC_VERIFY

#else //AC_COMPSIM

#define ac_init() \
  ac_init_opt( ac, av); \
  std::cerr << endl; \
  AC_SAY("Compiled simulator for program: " << appfilename); \
  ac_initialize();\

#endif //ndef AC_COMPSIM
 


#ifndef AC_COMPSIM

#define ac_start() \
  sc_initialize();\
  while (!ac_stop_flag) {\
    clk.write(1);\
    sc_cycle(clk_cycle/2);\
    clk.write(0);\
    sc_cycle(clk_cycle/2);\
  }

#else

#define ac_start() \
  ac_pc = ac_start_addr;\
  char **new_argv = new char*[ac];\
  new_argv[0] = appfilename;\
  for (int i=1; i<ac; i++) new_argv[i] = av[i];\
  void Execute(int,char **);\
  Execute(ac, new_argv);\
  delete [] new_argv;

#endif



/////////////////////////////////////////
//ArchC class for instructions signals //
//used in behavioral simulation        //
/////////////////////////////////////////
class ac_instr{

  unsigned  instr[AC_DEC_FIELD_NUMBER];

public:

  //!Constructor
  ac_instr( unsigned *im_instr ){

    if( im_instr )
      for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
        instr[j] = im_instr[j];
      }
    else
      for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
        instr[j] = 0;
      }
  }
    
  //!Constructor. Fields set to zero.
  ac_instr(){

    for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
      instr[j] = 0;
    }
  }

  //!Overloaded operators
  ac_instr & operator= ( const ac_instr &i){
    
    for( int j =0; j<AC_DEC_FIELD_NUMBER; j++){
      instr[j] = i.instr[j];
    }
    return *this;
  }
  
  friend bool operator== (const ac_instr & i1,
                          const ac_instr & i2){
    bool flag = true;
    
    for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
      flag = flag && (i1.instr[j] == i2.instr[j]);
    }
    
    return flag;
  }
  
  //Used by stream insertion operator
  void print (ostream & os) const{
    for( int j =0; j<AC_DEC_FIELD_NUMBER; j++)
      os << instr[j]<< ",";
    os << endl;
  }

  friend ostream& operator<< (ostream &os,
                              const ac_instr &i){
    i.print(os);
    return os;
  }
  
  //!Used in  trace functions.
  operator bool () const {return true;}

  //!Get field method.
  unsigned get( const int i ){ return instr[i];  }
 
  //!Put field method.
  void put(const unsigned data, const unsigned i ){ instr[i] = data;  }
};

//Struct used for decoded instructions cache
struct cache_item {

  bool valid;
  ac_instr* instr_p;
};

//Functions used to calculate the instr/s of the simulation
void InitStat();
void PrintStat();

//Initialize simulation
void ac_initialize();

//Stop simulation (may receive exit status)
void ac_stop(int status=0);


#endif //_ARCHC_H

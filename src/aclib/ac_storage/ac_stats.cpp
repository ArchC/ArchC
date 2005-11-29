/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC Statistics Library for the ArchC architecture simulators
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
/* Statistics class                                     */
/* Author:  Sandro Rigo                                 */
/*                                                      */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/
 

#include  "ac_stats.H"


//!Constructor.
ac_stats::ac_stats():INIT_STO_STATS{

  //!Initializing.
  for( int i = 0; i < AC_DEC_INSTR_NUMBER; i++)
    instr_table[i].count = 0;

  instr_executed = 0;
  init_stat();
}



//!Print simulation statistics report.
void ac_stats::print( ){

  int i;

  ac_sto_stats *pstats;

  output << flush;
  output << "********************************************" << endl;
  output << "*    ArchC Simulation Statistics Report    *" << endl;
  output << "********************************************" << endl;

  output << endl;

  output << "Simulation time: " << time << endl;
  output << "Total of Executed Instructions: " << instr_executed << endl;
  output << "Total of Executed Syscalls: " << syscall_executed << endl;

  output << endl;

#ifdef AC_CYCLE_RANGE
  output << "Cycle count estimates "<< endl;
  output << "Minimum number of cycles: " << ac_min_cycle_count << endl;
  output << "Maximum number of cycles: " << ac_max_cycle_count << endl;

  output << endl;
#endif

  output << "=> Instruction Set Statistics: " << endl << endl;

  for( i =1; i<= AC_DEC_INSTR_NUMBER; i++) {

      output << instr_table[i].name;
      output << " was executed " << instr_table[i].count << " times." << endl;
  }
  output << endl;

  output << "=> Storage Statistics: " << endl << endl;

  for ( pstats = head; pstats != NULL; pstats = pstats->next ){

    output << "Device " << pstats->get_name() << " was accessed " << pstats->get_accesses() << " times";

    if( pstats->get_misses() )
      {
        output << " and had " << pstats->get_misses() << " misses. " ;
        output.setf(ios::fixed);
        output.precision(2);
        output << "(Miss Rate: " << (float)100*pstats->get_misses()/pstats->get_accesses() << "%)" << endl;
      }
    else
      output << "." << endl;
  }
  
};

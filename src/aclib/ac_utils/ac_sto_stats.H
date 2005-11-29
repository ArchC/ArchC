/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC Storage Statistics Library for the ArchC architecture simulators
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
/* Storage Statistics class                             */
/* Author:  Sandro Rigo                                 */
/*                                                      */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

//!Storage Statistcs class.
class ac_sto_stats {

  char *name;
  long long accesses;
  long long misses;

public:

  ac_sto_stats *next;

  //!Constructor
  ac_sto_stats( char* n ): accesses(0), misses(0){ name = n;}

  void inc_accesses(){ accesses++;}
  void inc_accesses( int n ) { accesses+= n;}
  void inc_misses(){ misses++;}
  void inc_misses( int n ){ misses+=n;}

  int get_accesses() {return accesses;}
  int get_misses()  {return misses;}
  char* get_name()  {return name;}
};

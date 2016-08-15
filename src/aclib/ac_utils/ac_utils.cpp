/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_utils.cpp
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
 * @brief     ArchC support functions
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

#include "ac_utils.H"

#ifdef USE_GDB
#include "ac_gdb.H"
// extern AC_GDB *gdbstub;
#endif /* USE_GDB */

#ifdef AC_VERIFY

//Declaring co-verification message queue
// key_t key;
// int msqid;


#endif

// int ac_stop_flag = 0;
// int ac_exit_status = 0;

//Declaring trace variables
ofstream trace_file;
bool ac_do_trace;

//Name of the file containing the application to be loaded.
char *appfilename = NULL;
std::map<std::string, std::ofstream*> ac_cache_traces;


unsigned int convert_endian(unsigned int size, unsigned int num, bool match_endian)
{
  unsigned char *in = (unsigned char*) &num;
  unsigned int out = 0;

  if (! match_endian) {
    for(; size>0; size--) {
      out <<= 8;
      out |= in[0];
      in++;
    }
  }
  else {
    out = num;
  }

  return out;
}

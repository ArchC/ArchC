/**
 * @file      ac_log.H
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
 * @brief     Include file for ArchC log types
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
 
#ifndef _AC_LOG_H
#define _AC_LOG_H

#include <list>
#include <iostream>
#include <fstream>
#include <iomanip>

using std::list;
using std::setw;
using std::ostream;
using std::fstream;
using std::hex;
using std::dec;

/////////////////////////////////////////////////////////
/*!Structure used to keep track of changes on storage
   devices.               */
/////////////////////////////////////////////////////////
template <typename ac_word> class change_log {
public:

  unsigned addr;        //!<The address being written.
  ac_word value;            //!<New value assigned to the address.
  double time;          //!<Simulation time of this modification.



  //!Constructors
  change_log(): addr(0), value(0), time(0.0){}

  change_log( int a, ac_word  v, double t): addr(a), value(v), time(t){}

  //!Equal to operator overloaded.
  friend bool operator== (const change_log<ac_word>  & cl1,
			  const change_log<ac_word>  & cl2){
    bool flag = true;
    
    flag = flag && (cl1.addr == cl2.addr);
    flag = flag && (cl1.time == cl2.time);
    flag = flag && (cl1.value == cl2.value);
    
    return flag;
  }

  //!Less than operator overloaded.
  friend bool operator< (const change_log<ac_word>  & cl1,
			 const change_log<ac_word>  & cl2){
    
    if( cl1.time != cl2.time )
      return (cl1.time < cl2.time);
    else
      return (cl1.addr < cl2.addr);
    
  }

  //!Output operator overloaded.
  friend ostream& operator<< (ostream &os,
			const change_log<ac_word> &cl){

		os.fill(' ');
    os << "  " << setw(10) << hex << cl.addr 
			 << "    " << setw(10) << (int)cl.value 
			 << "    " << setw(12) << dec << cl.time;
    return os;
  }
  

  //!Saving into a given binary file
	void save( fstream &of ){

		char *buffer;
		int size = sizeof(ac_word) + sizeof(unsigned) + sizeof(double);
		buffer = new char[size];

		//Storing fields into the buffer
		*((int *)buffer) = addr; 
		*((ac_word *)(buffer+sizeof(unsigned))) = value; 
		*((double *)(buffer+sizeof(ac_word)+sizeof(unsigned))) = time; 

		of.write(buffer, size);
	}

};

#endif //_AC_LOG_H


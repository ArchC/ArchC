/**
 * @file      ac_storage.H
 * @author    Thiago Massariolli Sigrist
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
 *            TLM 2.0 time info: November, 2013
 *
 * @brief     Defines a class modeling a basic storage device (memory).
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

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_STORAGE_H_
#define _AC_STORAGE_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>
#include <systemc>

 
// SystemC includes

// ArchC includes
#include "ac_inout_if.H"

//////////////////////////////////////////////////////////////////////////////

// 'using' statements
using std::string;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// Models a basic storage device, used as main memory by default.
class ac_storage : public ac_inout_if {
private:
  ac_ptr data;
  string name;
  uint32_t size;

public:
  // constructor
  ac_storage(string nm, uint32_t sz);

  // destructor
  virtual ~ac_storage();

  // getters and setters
  void set_name(string n);

  string get_name() const;

  uint32_t get_size() const;

  void read(ac_ptr buf, uint32_t address,
		   int wordsize);

  void read(ac_ptr buf, uint32_t address,
		   int wordsize, int n_words);

  void write(ac_ptr buf, uint32_t address,
		    int wordsize);

  void write(ac_ptr buf, uint32_t address,
		    int wordsize, int n_words);


// Just for TLM2 support and compatibility
  void read(ac_ptr buf, uint32_t address,
         int wordsize,sc_core::sc_time &time_info);

  void read(ac_ptr buf, uint32_t address,
         int wordsize, int n_words,sc_core::sc_time &time_info);

  void write(ac_ptr buf, uint32_t address,
          int wordsize,sc_core::sc_time &time_info);

  void write(ac_ptr buf, uint32_t address,
          int wordsize, int n_words,sc_core::sc_time &time_info);





  /** 
   * Locks the device.
   * 
   */
   void lock(); 

  /** 
   * Unlocks the device.
   * 
   */
   void unlock();

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_STORAGE_H_

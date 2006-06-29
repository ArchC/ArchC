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
 *
 * @brief     Defines a class modeling a basic storage device (memory).
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_STORAGE_H_
#define _AC_STORAGE_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>

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

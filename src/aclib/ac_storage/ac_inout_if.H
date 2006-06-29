/**
 * @file      ac_inout_if.H
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
 * @date      Mon Sep 12 20:45:27 2005
 *
 * @brief     Defines an interface class with input/output methods.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_INOUT_IF_H_
#define _AC_INOUT_IF_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <stdint.h>
#include <string>

// SystemC includes

// ArchC includes
#include "ac_ptr.H"

//////////////////////////////////////////////////////////////////////////////

// 'using' statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// ac_inout_if is a simple interface that contains read, write and lock
/// methods. It is used mainly to access non-memory external devices.
/// For memory devices, convenience methods for binary/array/ELF loading
/// and for dumping are present on the ac_memport wrapper.
class ac_inout_if {
public:

  /** 
   * Reads a single word.
   * 
   * @param buf Buffer into which the word will be copied.
   * @param address Address from where the word will be read.
   * @param wordsize Word size in bits.
   * 
   */
  virtual void read(ac_ptr buf, uint32_t address,
		    int wordsize) = 0;

  /** 
   * Reads multiple words.
   * 
   * @param buf Buffer into which the words will be copied.
   * @param address Address from where the words will be read.
   * @param wordsize Word size in bits.
   * @param n_words Number of words to be read.
   * 
   */
  virtual void read(ac_ptr buf, uint32_t address,
		    int wordsize, int n_words) = 0;

  /** 
   * Writes a single word.
   * 
   * @param buf Buffer from which the word will be copied.
   * @param address Address to where the word will be written.
   * @param wordsize Word size in bits.
   *
   */
  virtual void write(ac_ptr buf, uint32_t address,
		     int wordsize) = 0;

  /** 
   * Writes multiple words.
   * 
   * @param buf Buffer from which the words will be copied.
   * @param address Address to where the words will be written.
   * @param wordsize Word size in bits.
   * @param n_words Number of words to be written.
   * 
   */
  virtual void write(ac_ptr buf, uint32_t address,
		     int wordsize, int n_words) = 0;

  virtual std::string get_name() const = 0;

  virtual uint32_t get_size() const = 0;

  /** 
   * Locks the device.
   * 
   */
   virtual void lock() = 0;

  /** 
   * Unlocks the device.
   * 
   */
   virtual void unlock() = 0;

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_INOUT_IF_H_

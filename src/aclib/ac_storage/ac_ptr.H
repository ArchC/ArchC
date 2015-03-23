/**
 * @file      ac_ptr.H
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
 * @date      Tue Sep 13 19:05:35 2005
 *
 * @brief     Defines a convenience union for pointers to integer types.
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

#ifndef _AC_PTR_H_
#define _AC_PTR_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <stdint.h>

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// 'using' statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// Convenience union for pointers to integer types.
union ac_ptr {
  uint8_t* ptr8;
  uint16_t* ptr16;
  uint32_t* ptr32;
  uint64_t* ptr64;

  inline ac_ptr() {}

  inline ac_ptr(uint8_t* p) {
    ptr8 = p;
  }

  inline ac_ptr(uint16_t* p) {
    ptr16 = p;
  }

  inline ac_ptr(uint32_t* p) {
    ptr32 = p;
  }

  inline ac_ptr(uint64_t* p) {
    ptr64 = p;
  }

  inline ac_ptr& operator=(uint8_t* rhs) {
    ptr8 = rhs;
    return *this;
  }

  inline ac_ptr& operator=(uint16_t* rhs) {
    ptr16 = rhs;
    return *this;
  }

  inline ac_ptr& operator=(uint32_t* rhs) {
    ptr32 = rhs;
    return *this;
  }

  inline ac_ptr& operator=(uint64_t* rhs) {
    ptr64 = rhs;
    return *this;
  }

  inline operator uint8_t*() {
    return ptr8;
  }

  inline operator uint16_t*() {
    return ptr16;
  }

  inline operator uint32_t*() {
    return ptr32;
  }

  inline operator uint64_t*() {
    return ptr64;
  }

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_PTR_H_

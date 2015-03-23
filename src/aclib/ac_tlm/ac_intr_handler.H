/**
 * @file      ac_intr_handler.H
 * @author    Thiago Massariolli Sigrist
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @version   2.0alpha1
 * @date      Fri, 24 Mar 2006 17:16:37 -0300
 * 
 * @brief     Defines the interface for the ArchC interrupt handler.
 * 
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
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

#ifndef _AC_INTR_HANDLER_H_
#define _AC_INTR_HANDLER_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <stdint.h>

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// using statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// ArchC standard interrupt handler interface.
class ac_intr_handler {
public:

  /**
   * Interrupt handler method
   *
   * @param value Value received by the port, passed to the method.
   *
   */
  virtual void handle(uint32_t value) = 0;
};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_INTR_HANDLER_H_


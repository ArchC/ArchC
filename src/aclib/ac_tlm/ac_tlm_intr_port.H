/**
 * @file      ac_tlm_intr_port.H
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
 * @date      Fri, 24 Mar 2006 16:34:45 -0300
 * 
 * @brief     Defines the ArchC TLM interrupt (slave) export.
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

#ifndef _AC_TLM_INTR_PORT_H_
#define _AC_TLM_INTR_PORT_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>

// SystemC includes
#include <systemc.h>

// ArchC includes
#include "ac_inout_if.H"
#include "ac_tlm_protocol.H"
#include "ac_intr_handler.H"

//////////////////////////////////////////////////////////////////////////////

// using statements
using std::string;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// ArchC TLM Interrupt port class.
class ac_tlm_intr_port : public ac_tlm_transport_if,
                         public sc_export<ac_tlm_transport_if> {
private:
  ac_intr_handler& handler;

public:
  string name;

  /**
   * Default constructor.
   *
   * @param nm Port name.
   * @param hnd Interrupt handler for this port.
   *
   */
  explicit ac_tlm_intr_port(char const* nm, ac_intr_handler& hnd);

  /**
   * TLM transport function.
   *
   * @param req ArchC TLM protocol request packet.
   *
   * @return ArchC TLM protocol response packet.
   *
   */
  ac_tlm_rsp transport(const ac_tlm_req& req);

  /**
   * Default (virtual) destructor.
   * @return Nothing.
   */
  virtual ~ac_tlm_intr_port();

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_TLM_INTR_PORT_H_


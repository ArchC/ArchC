/**
 * @file      ac_tlm_protocol.H
 * @author    Thiago Massariolli Sigrist
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @version   2.0beta2
 * @date      Tue, 13 Dec 2005 20:09:49 -0200
 * 
 * @brief     Defines the ArchC TLM packet structure.
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

#ifndef _AC_TLM_PROTOCOL_H_
#define _AC_TLM_PROTOCOL_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <stdint.h>

// SystemC includes
#include <systemc.h>
#include <tlm.h>

// ArchC includes

 // TLM 2.0 payload structure
#include "ac_tlm2_payload.H"

//////////////////////////////////////////////////////////////////////////////

// using statements
using tlm::tlm_transport_if;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// ArchC TLM request type.
enum ac_tlm_req_type {
  READ, WRITE, LOCK, UNLOCK, REQUEST_COUNT
};

/// ArchC TLM response status.
enum ac_tlm_rsp_status {
  ERROR, SUCCESS
};

/// ArchC TLM request packet.
struct ac_tlm_req {
  ac_tlm_req_type type;
  int dev_id;
  uint32_t addr;
  union {
    uint32_t data;
    uint16_t data16;
    uint8_t data8;
  };
};

/// ArchC TLM response packet.
struct ac_tlm_rsp {
  ac_tlm_rsp_status status;
  ac_tlm_req_type req_type;
  union {
    uint32_t data;
    uint16_t data16;
    uint8_t data8;
  };
};

/// ArchC TLM 1.0 transport interface type.
typedef tlm_transport_if<ac_tlm_req, ac_tlm_rsp> ac_tlm_transport_if;

//////////////////////////////////////////////////////////////////////////////


// TLM 2 payload and blocking interface
using  tlm::tlm_blocking_transport_if;    // TLM 2 blocking interface
using  tlm::tlm_fw_nonblocking_transport_if;  // TLM 2 forwarding non-blocking interface
using  tlm::tlm_bw_nonblocking_transport_if;  // TLM 2 backarding non-blocking interface
using  tlm::tlm_command;

typedef ac_tlm2_payload tlm_payload_type;
typedef tlm_blocking_transport_if<ac_tlm2_payload> ac_tlm2_blocking_transport_if;
typedef tlm_fw_nonblocking_transport_if<ac_tlm2_payload> ac_tlm2_fw_nonblocking_transport_if;
typedef tlm_bw_nonblocking_transport_if<ac_tlm2_payload> ac_tlm2_bw_nonblocking_transport_if;

#endif // _AC_TLM_PROTOCOL_H_






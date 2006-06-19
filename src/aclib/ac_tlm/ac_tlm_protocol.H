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
  uint32_t data;
};

/// ArchC TLM response packet.
struct ac_tlm_rsp {
  ac_tlm_rsp_status status;
  ac_tlm_req_type req_type;
  uint32_t data;
};

/// ArchC TLM transport interface type.
typedef tlm_transport_if<ac_tlm_req, ac_tlm_rsp> ac_tlm_transport_if;

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_TLM_PROTOCOL_H_


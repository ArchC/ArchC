/**
 * @file      ac_tlm2_intr_port.cpp
 * @author    Liana Duenha
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @date      November, 2013
 * 
 * @brief     ArchC TLM 2.0 interrupt (slave) port implementation.
 * 
 * 
 */

//////////////////////////////////////////////////////////////////////////////

// Standard includes

// SystemC includes

// ArchC includes
#include<tlm.h>
#include "ac_tlm2_intr_port.H"
#include "ac_tlm2_payload.H"

//////////////////////////////////////////////////////////////////////////////

// using statements

using tlm::TLM_WRITE_COMMAND;
using tlm::TLM_READ_COMMAND;
//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

// Constructors

/**
 * Default constructor.
 *
 * @param nm Port name.
 * @param hnd Interrupt handler for this port.
 *
 */
ac_tlm2_intr_port::ac_tlm2_intr_port(char const* nm, ac_intr_handler& hnd) :
  handler(hnd),
  name(nm)
  {
    bind(*this);
  }

//////////////////////////////////////////////////////////////////////////////

// Methods

/**
 * TLM 2.0 blocking transport function.
 *
 * @param req ArchC TLM protocol request packet.
 *
 * @return ArchC TLM protocol response packet.
 *
 */




void ac_tlm2_intr_port::b_transport(ac_tlm2_payload &payload, sc_core::sc_time &time_info)
{
  //unsigned char *data = (unsigned char*) payload.get_data_ptr();  
  tlm_command command = payload.get_command();

  unsigned char *p = payload.get_data_ptr();
  uint64_t addr = payload.get_address();

  uint32_t *T = reinterpret_cast<uint32_t*>(p);
  uint32_t data_p= T[0];

  switch( command )
  {
    case TLM_WRITE_COMMAND:    
      handler.handle(data_p,addr);       
      payload.set_response_status(tlm::TLM_OK_RESPONSE);
      break;
    
    default : 
      break; 
    }
}

//////////////////////////////////////////////////////////////////////////////

// Destructors

/**
 * Default (virtual) destructor.
 * @return Nothing.
 */
ac_tlm2_intr_port::~ac_tlm2_intr_port() {}

//////////////////////////////////////////////////////////////////////////////


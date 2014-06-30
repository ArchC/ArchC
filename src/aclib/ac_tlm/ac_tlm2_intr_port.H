/**
 * @file      ac_tlm2_intr_port.H
 * @author    Liana Duenha
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @version   2.0
 * @date      November, 2013
 * 
 * @brief     Defines the ArchC TLM2 interrupt (slave) export.
 * 
 * 
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_TLM2_INTR_PORT_H_
#define _AC_TLM2_INTR_PORT_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>

// SystemC includes
#include <systemc.h>

// ArchC includes
#include<tlm.h>
#include "ac_inout_if.H"
#include "ac_tlm_protocol.H"
#include "ac_intr_handler.H"
#include "ac_tlm2_payload.H"

//////////////////////////////////////////////////////////////////////////////

// using statements
using std::string;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// ArchC TLM Interrupt port class.
class ac_tlm2_intr_port : public ac_tlm2_blocking_transport_if,
                         public sc_export<ac_tlm2_blocking_transport_if> {
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
  explicit ac_tlm2_intr_port(char const* nm, ac_intr_handler& hnd);

  /**
   * TLM2 blocking transport function.
   *
   *
   */
  // ac_tlm_rsp transport(const ac_tlm_req& req);
  void b_transport(ac_tlm2_payload &, sc_core::sc_time &);
  /**
   * Default (virtual) destructor.
   * @return Nothing.
   */
  virtual ~ac_tlm2_intr_port();

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_TLM_INTR_PORT_H_


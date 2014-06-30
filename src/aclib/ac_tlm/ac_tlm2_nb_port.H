/**
 * @file      ac_tlm2_nb_port.H
 * @author    Liana Duenha
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @version   
 * @date      22, October, 2012
 * 
 * @brief     Defines the ArchC TLM 2.0  non-blocking port.
 *
 * @attention Copyright (C) 2002-2012 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_TLM2_NB_PORT_H_
#define _AC_TLM2_NB_PORT_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>

// SystemC includes
#include <systemc.h>

// TLM includes
#include <tlm.h>
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"


// ArchC includes

#include "ac_inout_if.H"
#include "ac_tlm_protocol.H"
#include "ac_tlm_dev_id.H"


//////////////////////////////////////////////////////////////////////////////

// using statements
using std::string;


//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// ArchC TLM initiator port class.    
class ac_tlm2_nb_port : 
		     public ac_inout_if,
		     public ac_tlm_dev_id {

private:
    /// Persistent payload used in read/write transactions
    
    ac_tlm2_payload *payload_global;     /* PAYLOAD   */
    

public:
  string name;
  uint32_t size;
  sc_event wake_up;
  
  tlm_utils::simple_initiator_socket<ac_tlm2_nb_port> LOCAL_init_socket;
  tlm::tlm_sync_enum  nb_transport_bw(ac_tlm2_payload &, tlm::tlm_phase &, sc_core::sc_time &);


  
  explicit ac_tlm2_nb_port(char const* name, uint32_t sz);


  virtual ~ac_tlm2_nb_port();

  
  virtual void read(ac_ptr buf, uint32_t address,
		    int wordsize,sc_core::sc_time &time_info);

  
  virtual void read(ac_ptr buf, uint32_t address, int wordsize, int n_words,sc_core::sc_time &time_info);

  
  virtual void write(ac_ptr buf, uint32_t address,
		     int wordsize,sc_core::sc_time &time_info);

  
  virtual void write(ac_ptr buf, uint32_t address,
		     int wordsize, int n_words,sc_core::sc_time &time_info);


  // Just for compatibility
  virtual void read(ac_ptr buf, uint32_t address, int wordsize)
  {
  	  sc_core::sc_time time_info(0,SC_NS);
  	  read(buf, address, wordsize,time_info);
  }
  virtual void read(ac_ptr buf, uint32_t address, int wordsize, int n_words)
  {
  	  sc_core::sc_time time_info(0,SC_NS);
  	  read(buf, address, wordsize,n_words,time_info);

  }
  virtual void write(ac_ptr buf, uint32_t address, int wordsize)
  {
  	  sc_core::sc_time time_info(0,SC_NS);
  	  write(buf, address, wordsize,time_info);
  }
  virtual void write(ac_ptr buf, uint32_t address,int wordsize, int n_words)
  {
  	  sc_core::sc_time time_info(0,SC_NS);
  	  write(buf, address, wordsize, n_words,time_info);
  }


  virtual string get_name() const;

  virtual uint32_t get_size() const;
  

  /** 
   * Locks the device.
   * 
   */
   virtual void lock();

  /** 
   * Unlocks the device.
   * 
   */
   virtual void unlock();

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_TLM2_NB_PORT_H_



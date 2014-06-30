/**
 * @file      ac_tlm2_port.H

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
 * @brief     Defines the ArchC TLM 2.0  port.
 *
 * @attention Copyright (C) 2002-2012 --- The ArchC Team
 *
 */



//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_TLM2_PORT_H_
#define _AC_TLM2_PORT_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>

// SystemC includes
#include <systemc.h>

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

/// ArchC TLM initiator port class.    /**** retirei public ac_inout_if,  ****//
class ac_tlm2_port : public sc_port<ac_tlm2_blocking_transport_if>,
                     public ac_inout_if,
                     public ac_tlm_dev_id {

private:
    /// Persistent payload used in read/write transactions
    ac_tlm2_payload* payload;     /* PAYLOAD   */

public:
  string name;
  uint32_t size;



  /** 
   * Default constructor.
   */
  explicit ac_tlm2_port(char const* name, uint32_t sz);

  /**
   * Default (virtual) destructor.
   * @return Nothing.
   */
  virtual ~ac_tlm2_port();

  /** 
   * Reads a single word.
   * 
   */
  virtual void read(ac_ptr buf, uint32_t address,
		    int wordsize,sc_core::sc_time &time_info);

  /** 
   * Reads multiple words.
   * 
   */
  
  virtual void read(ac_ptr buf, uint32_t address, int wordsize, int n_words,sc_core::sc_time &time_info);

  /** 
   * Writes a single word.
   * 
   */
  virtual void write(ac_ptr buf, uint32_t address,
		     int wordsize,sc_core::sc_time &time_info);

  /** 
   * Writes multiple words.
   * 
   */
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

#endif // _AC_TLM2_PORT_H_



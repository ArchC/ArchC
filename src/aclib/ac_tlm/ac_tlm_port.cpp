/**
 * @file      ac_tlm_port.cpp
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
 * @date      Tue, 13 Dec 2005 20:09:00 -0200
 * 
 * @brief     ArchC TLM initiator port class implementation.
 * 
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 * 
 */

//////////////////////////////////////////////////////////////////////////////

// Standard includes

// SystemC includes

// ArchC includes
#include "ac_tlm_port.H"

//////////////////////////////////////////////////////////////////////////////

// using statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

// Constructors

/** 
 * Default constructor.
 * 
 * @param size Size or address range of the element to be attached.
 * 
 */
ac_tlm_port::ac_tlm_port(char const* nm, uint32_t sz) : name(nm), size(sz) {}

//////////////////////////////////////////////////////////////////////////////

// Methods

/** 
 * Reads a single word.
 * 
 * @param buf Buffer into which the word will be copied.
 * @param address Address from where the word will be read.
 * @param wordsize Word size in bits.
 * 
 */
void ac_tlm_port::read(ac_ptr buf, uint32_t address, int wordsize) {
  ac_tlm_req req;
  ac_tlm_rsp rsp;

  req.type = READ;
  req.addr = address;
  req.data = 0ULL;

  rsp = (*this)->transport(req);

  if (rsp.status == SUCCESS) {
    switch (wordsize) {
    case 8:
      *(buf.ptr8) = ((uint8_t*)&rsp.data)[0];
      break;
    case 16:
      *(buf.ptr16) = ((uint16_t*)&rsp.data)[0];
      break;
    case 32:
      *(buf.ptr32) = ((uint32_t*)&rsp.data)[0];
      break;
    case 64:
      *(buf.ptr64) = rsp.data;
      break;
    default:
      break;
    }
  }
}

/** 
 * Reads multiple words.
 * 
 * @param buf Buffer into which the words will be copied.
 * @param address Address from where the words will be read.
 * @param wordsize Word size in bits.
 * @param n_words Number of words to be read.
 * 
 */
void ac_tlm_port::read(ac_ptr buf, uint32_t address,
		       int wordsize, int n_words) {
  ac_tlm_req req;
  ac_tlm_rsp rsp;

  req.type = READ;

  switch (wordsize) {
  case 8:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + i;
      req.data = 0ULL;
      
      rsp = (*this)->transport(req);
      
      if (rsp.status == SUCCESS) {
	for (int j = 0; (i < n_words) && (j < 4); i++, j++) { 
	  (buf.ptr8)[i] = ((uint8_t*)&rsp.data)[j];
	}
        i--;
      }
    }
    break;
  case 16:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint16_t));
      req.data = 0ULL;
      
      rsp = (*this)->transport(req);
      
      if (rsp.status == SUCCESS) {
	for (int j = 0; (i < n_words) && (j < 2); i++, j++) { 
	  (buf.ptr16)[i] = ((uint16_t*)&rsp.data)[j];
	}
        i--;
      }
    }
    break;
  case 32:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint32_t));
      req.data = 0ULL;
      
      rsp = (*this)->transport(req);
      
      if (rsp.status == SUCCESS) {
	for (int j = 0; (i < n_words) && (j < 1); i++, j++) { 
	  (buf.ptr32)[i] = ((uint32_t*)&rsp.data)[j];
	}
        i--;
      }
    }
    break;
  case 64:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint64_t));
      req.data = 0ULL;
      
      rsp = (*this)->transport(req);
      
      if (rsp.status == SUCCESS) {
	(buf.ptr64)[i] = rsp.data;
      }
    }
    break;
  default:
    break;
  }
}

/** 
 * Writes a single word.
 * 
 * @param buf Buffer from which the word will be copied.
 * @param address Address to where the word will be written.
 * @param wordsize Word size in bits.
 *
 */
void ac_tlm_port::write(ac_ptr buf, uint32_t address, int wordsize) {
  ac_tlm_req req;
  ac_tlm_rsp rsp;

//   req.type = WRITE;
//   req.addr = address;

  // It would be necessary to read 64 bits and mix the bits with the smaller
  // words that will be written

  // Rodolfo: This should be not necessary. Trying with only 32 bits...
  switch (wordsize) {
  case 8:
    req.type = READ;
    req.addr = address;
    rsp = (*this)->transport(req);

    req.type = WRITE;
    req.data = rsp.data;
    ((uint8_t*)&(req.data))[0] = *(buf.ptr8);
    rsp = (*this)->transport(req);
    break;
  case 16:
    req.type = READ;
    req.addr = address;
    rsp = (*this)->transport(req);

    req.type = WRITE;
    req.data = rsp.data;
    ((uint16_t*)&(req.data))[0] =
      *(buf.ptr16);
    rsp = (*this)->transport(req);
    break;
  case 32:
 //   req.type = READ;
    req.addr = address;
 //   rsp = (*this)->transport(req);

    req.type = WRITE;
    req.data = rsp.data;
    ((uint32_t*)&(req.data))[0] =
      *(buf.ptr32);
    rsp = (*this)->transport(req);
    break;

// This is not a 64-bit operation!
  case 64:
    req.type = WRITE;
    req.addr = address;
    req.data = *(buf.ptr64);
    rsp = (*this)->transport(req);
    break;
  default:
    break;
  }
}

/** 
 * Writes multiple words.
 * 
 * @param buf Buffer from which the words will be copied.
 * @param address Address to where the words will be written.
 * @param wordsize Word size in bits.
 * @param n_words Number of words to be written.
 * 
 */
void ac_tlm_port::write(ac_ptr buf, uint32_t address,
			int wordsize, int n_words) {
  ac_tlm_req req;
  ac_tlm_rsp rsp;

  switch (wordsize) {
  case 8:
    for (int i = 0; i < n_words; i++) {
      req.type = READ;
      req.addr = address + i;
      req.data = 0ULL;
      rsp = (*this)->transport(req);

      req.type = WRITE;
      req.data = rsp.data;

      for (int j = 0; (i < n_words) && (j < 4); j++, i++) {
	((uint8_t*)&req.data)[j] = (buf.ptr8)[i];
      }
      i--;
      (*this)->transport(req);
    }
    break;
  case 16:
    for (int i = 0; i < n_words; i++) {
      req.type = READ;
      req.addr = address + (i * sizeof(uint16_t));
      req.data = 0ULL;
      rsp = (*this)->transport(req);

      req.type = WRITE;
      req.data = rsp.data;

      for (int j = 0; (i < n_words) && (j < 2); j++, i++) {
	((uint16_t*)&req.data)[j] = (buf.ptr16)[i];
      }
      i--;
      (*this)->transport(req);
    }
    break;
  case 32:
    for (int i = 0; i < n_words; i++) {
//      req.type = READ;
      req.addr = address + (i * sizeof(uint32_t));
      req.data = 0ULL;
//      rsp = (*this)->transport(req);

      req.type = WRITE;
//      req.data = rsp.data;
      req.data = buf.ptr32[i];

//      for (int j = 0; (i < n_words) && (j < 1); j++, i++) {
//        ((uint32_t*)&req.data)[j] = (buf.ptr32)[i];
//      }
//      i--;
      (*this)->transport(req);
    }
    break;
  case 64:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint64_t));
      req.data = (buf.ptr64)[i];
      (*this)->transport(req);
    }
    break;
  default:
    break;
  }
}

string ac_tlm_port::get_name() const {
  return name;
}

uint32_t ac_tlm_port::get_size() const {
  return size;
}

/** 
 * Locks the device.
 * 
 */
void ac_tlm_port::lock()
{
  ac_tlm_req req;
  req.type = LOCK;
  req.dev_id = dev_id_;
  (*this)->transport(req);
}

/** 
 * Unlocks the device.
 * 
 */
void ac_tlm_port::unlock()
{
  ac_tlm_req req;
  req.type = UNLOCK;
  req.dev_id = dev_id_;
  (*this)->transport(req);
}

//////////////////////////////////////////////////////////////////////////////

// Destructors

/**
 * Default (virtual) destructor.
 * @return Nothing.
 */
ac_tlm_port::~ac_tlm_port() {}

//////////////////////////////////////////////////////////////////////////////


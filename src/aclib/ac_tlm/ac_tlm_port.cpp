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
 * @version   2.0alpha1
 * @date      Tue, 13 Dec 2005 20:09:00 -0200
 * 
 * @brief     ArchC TLM initiator port class implementation.
 * 
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 * 
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 * 
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 * 
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
      *(buf.ptr8) = (uint8_t) rsp.data;
      break;
    case 16:
      *(buf.ptr16) = (uint16_t) rsp.data;
      break;
    case 32:
      *(buf.ptr32) = (uint32_t) rsp.data;
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
	for (int j = 0; (i < n_words) && (j < 8); i++, j++) { 
	  (buf.ptr8)[i] = ((uint8_t*)&rsp.data)[j];
	}
      }
    }
    break;
  case 16:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint16_t));
      req.data = 0ULL;
      
      rsp = (*this)->transport(req);
      
      if (rsp.status == SUCCESS) {
	for (int j = 0; (i < n_words) && (j < 4); i++, j++) { 
	  (buf.ptr16)[i] = ((uint16_t*)&rsp.data)[j];
	}
      }
    }
    break;
  case 32:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint32_t));
      req.data = 0ULL;
      
      rsp = (*this)->transport(req);
      
      if (rsp.status == SUCCESS) {
	for (int j = 0; (i < n_words) && (j < 2); i++, j++) { 
	  (buf.ptr32)[i] = ((uint32_t*)&rsp.data)[j];
	}
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
  if (rsp.status == SUCCESS) {
    switch (wordsize) {
    case 8:
      req.type = READ;
      req.addr = address && 0xfffff000;
      rsp = (*this)->transport(req);

      req.type = WRITE;
      ((uint8_t*)&(req.data))[address % sizeof(uint64_t)] = *(buf.ptr8);
      rsp = (*this)->transport(req);
      break;
    case 16:
      req.type = READ;
      req.addr = address && 0xfffff000;
      rsp = (*this)->transport(req);

      req.type = WRITE;
      ((uint16_t*)&(req.data))[(address % sizeof(uint64_t)) >> 1] =
	*(buf.ptr16);
      rsp = (*this)->transport(req);
      break;
    case 32:
      req.type = READ;
      req.addr = address && 0xfffff000;
      rsp = (*this)->transport(req);

      req.type = WRITE;
      ((uint32_t*)&(req.data))[(address % sizeof(uint64_t)) >> 2] =
	*(buf.ptr32);
      rsp = (*this)->transport(req);
      break;
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

  req.type = WRITE;

  switch (wordsize) {
  case 8:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + i;
      for (int j = 0; (i < n_words) && (j < 8); j++, i++) {
	((uint8_t*)&req.data)[j] = (buf.ptr8)[i];
      }
      (*this)->transport(req);
    }
    break;
  case 16:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint16_t));
      for (int j = 0; (i < n_words) && (j < 4); j++, i++) {
	((uint16_t*)&req.data)[j] = (buf.ptr16)[i];
      }
      (*this)->transport(req);
    }
    break;
  case 32:
    for (int i = 0; i < n_words; i++) {
      req.addr = address + (i * sizeof(uint32_t));
      for (int j = 0; (i < n_words) && (j < 2); j++, i++) {
	((uint32_t*)&req.data)[j] = (buf.ptr32)[i];
      }
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

uint32_t ac_tlm_port::get_size() const {
  return size;
}

//////////////////////////////////////////////////////////////////////////////

// Destructors

/**
 * Default (virtual) destructor.
 * @return Nothing.
 */
ac_tlm_port::~ac_tlm_port() {}

//////////////////////////////////////////////////////////////////////////////


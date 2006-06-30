/**
 * @file      ac_storage.cpp
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include "ac_storage.H"

// constructor
ac_storage::ac_storage(string nm, uint32_t sz) :
  name(nm),
  size(sz) {
  data.ptr8 = new unsigned char[sz];
}

// destructor
ac_storage::~ac_storage() {
  delete[] data.ptr8;
}

// getters and setters
void ac_storage::set_name(string n) {
  name = n;
}

string ac_storage::get_name() const {
  return name;
}

uint32_t ac_storage::get_size() const {
  return size;
}

void ac_storage::read(ac_ptr buf, uint32_t address,
		      int wordsize) {
  switch (wordsize) {
  case 8: { // unsigned char
    *(buf.ptr8) = (data.ptr8)[address];
    break;
  }
  case 16: { // unsigned short
    *(buf.ptr16) = *((uint16_t*) (data.ptr8 + address));
    break;
  }
  case 32: { // unsigned int
    *(buf.ptr32) = *((uint32_t*) (data.ptr8 + address));
    break;
  }
  case 64: { // unsigned long long
    *(buf.ptr64) = *((uint64_t*) (data.ptr8 + address));
    break;
  }
  default: // weird size
    break;
  }
}

void ac_storage::read(ac_ptr buf, uint32_t address,
		      int wordsize, int n_words) {
  switch (wordsize) {
  case 8: { // unsigned char
    for (int i = 0; i < n_words; i++)
      (buf.ptr8)[i] = (data.ptr8)[address + i];
    break;
  }      
  case 16: { // unsigned short
    for (int i = 0; i < n_words; i++)
      (buf.ptr16)[i] = (data.ptr16)[address / 2 + i];
    break;
  }
  case 32: { // unsigned int
    for (int i = 0; i < n_words; i++)
      (buf.ptr32)[i] = (data.ptr32)[address / 4 + i];
    break;
  }
  case 64: { // unsigned long long
    for (int i = 0; i < n_words; i++)
      (buf.ptr64)[i] = (data.ptr64)[address / 8 + i];
    break;
  }
  default: // weird size
    break;
  }
}

void ac_storage::write(ac_ptr buf, uint32_t address,
		       int wordsize) {
  switch (wordsize) {
  case 8: { // unsigned char
    (data.ptr8)[address] = *(buf.ptr8);
    break;
  }
  case 16: { // unsigned short
    *((uint16_t*) (data.ptr8 + address)) = *(buf.ptr16);
    break;
  }
  case 32: { // unsigned int
    *((uint32_t*) (data.ptr8 + address)) = *(buf.ptr32);
    break;
  }
  case 64: { // unsigned long long
    *((uint64_t*) (data.ptr8 + address)) = *(buf.ptr64);
    break;
  }
  default: // weird size
    break;
  }
}

void ac_storage::write(ac_ptr buf, uint32_t address,
		       int wordsize, int n_words) {
  switch (wordsize) {
  case 8: { // unsigned char
    for (int i = 0; i < n_words; i++)
      (data.ptr8)[address + i] = (buf.ptr8)[i];
    break;
  }
  case 16: { // unsigned short
    for (int i = 0; i < n_words; i++)
      (data.ptr16)[address / 2 + i] = (buf.ptr16)[i];
    break;
  }
  case 32: { // unsigned int
    for (int i = 0; i < n_words; i++)
      (data.ptr32)[address / 4 + i] = (buf.ptr32)[i];
    break;
  }
  case 64: { // unsigned long long
    for (int i = 0; i < n_words; i++)
      (data.ptr64)[address / 8 + i] = (buf.ptr64)[i];
    break;
  }
  default: // weird size
    break;
  }
}

/** 
 * Locks the device.
 * 
 */
void ac_storage::lock()
{} // empty == there's no point locking the internal storage

/** 
 * Unlocks the device.
 * 
 */
void ac_storage::unlock()
{} // empty == there's no point locking the internal storage

//////////////////////////////////////////////////////////////////////////////


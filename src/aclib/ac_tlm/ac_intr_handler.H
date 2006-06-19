/**
 * @file      ac_intr_handler.H
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
 * @date      Fri, 24 Mar 2006 17:16:37 -0300
 * 
 * @brief     Defines the interface for the ArchC interrupt handler.
 * 
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 * 
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_INTR_HANDLER_H_
#define _AC_INTR_HANDLER_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <stdint.h>

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// using statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// ArchC standard interrupt handler interface.
class ac_intr_handler {
public:

  /**
   * Interrupt handler method
   *
   * @param value Value received by the port, passed to the method.
   *
   */
  virtual void handle(uint32_t value) = 0;
};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_INTR_HANDLER_H_


/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC Tick Interface Library for the ArchC architecture simulators
    Copyright (C) 2002-2005  The ArchC Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

/********************************************************/
/* Ticker interface for the register class              */
/* Author:  Marilia Felippe Chiozo                      */
/*                                                      */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

#ifndef _AC_TICKER_INTERFACE_H_
#define _AC_TICKER_INTERFACE_H_

// Standard includes.
#include <list>

class ac_ticker_interface
{
 protected:
  /// List of all tickers.
  static std::list<ac_ticker_interface*> tickers_list;
  /// Pointer to self in the list.
  std::list<ac_ticker_interface*>::iterator this_ticker;
  virtual void tick();
 public:
  static void mass_update();
  ac_ticker_interface();
  virtual ~ac_ticker_interface();
};

#endif // _AC_TICKER_INTERFACE_H

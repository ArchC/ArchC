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
/* Tick interface for classes that need "ticking".      */
/* Author:  Marilia Felippe Chiozo                      */
/*                                                      */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

#include "ac_ticker_interface.H"

std::list<ac_ticker_interface*> ac_ticker_interface::tickers_list;

void ac_ticker_interface::tick()
{
 return;
}

void ac_ticker_interface::mass_update()
{
 std::list<ac_ticker_interface*>::iterator i;

 for (i = tickers_list.begin(); i != tickers_list.end(); i++)
  (*i)->tick();
 return;
}

ac_ticker_interface::ac_ticker_interface()
{
 this_ticker = tickers_list.insert(tickers_list.end(), this);
 return;
}

ac_ticker_interface::~ac_ticker_interface()
{
 tickers_list.erase(this_ticker);
 return;
}

/**
 * @file      ac_printable_stats.H
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
 * @date      Thu, 04 May 2006 14:31:11 -0300
 *
 * @brief     Declares an interface for printable statistics classes.
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

#ifndef AC_PRINTABLE_STATS_H
#define AC_PRINTABLE_STATS_H

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <iostream>

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// using statements
using std::ostream;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// Interface for printable statistics classes.
class ac_printable_stats {
  public:
    virtual void print_stats(ostream& os) = 0;
};

//////////////////////////////////////////////////////////////////////////////

#endif // AC_PRINTABLE_STATS_H


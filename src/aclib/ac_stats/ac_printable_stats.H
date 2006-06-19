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


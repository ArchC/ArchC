/**
 * @file      ac_stats_base.H
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
 * @date      Wed, 03 May 2006 18:57:33 -0300
 *
 * @brief     Defines a base class for ArchC statistics.
 *
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef AC_STATS_BASE_H
#define AC_STATS_BASE_H

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <list>
#include <iostream>

// SystemC includes

// ArchC includes
#include "ac_printable_stats.H"

//////////////////////////////////////////////////////////////////////////////

// using statements
using std::list;
using std::ostream;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// Base class for managed statistics classes.
class ac_stats_base : public ac_printable_stats {
  private:
    static list<ac_stats_base*> list_of_stats_; //< All ac_stats instances.
    
  public:
    /// Default constructor.
    ac_stats_base();

    /// Prints info of all instances.
    static void print_all_stats(ostream& os);

    /// Prints info of this ac_stats_instance.
    virtual void print_stats(ostream& os) = 0;

    /// Virtual destructor.
    virtual ~ac_stats_base();
};

//////////////////////////////////////////////////////////////////////////////

#endif // AC_STATS_BASE_H


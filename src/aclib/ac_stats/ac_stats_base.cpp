/**
 * @file      ac_stats_base.cpp
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
 * @date      Wed, 03 May 2006 19:02:25 -0300
 * 
 * @brief     Defines the members of a base class for ArchC statistics.
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

// Standard includes

// SystemC includes

// ArchC includes
#include "ac_stats_base.H"

//////////////////////////////////////////////////////////////////////////////

// using statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

// Constructors

/// ac_stats_base default constructor
ac_stats_base::ac_stats_base()
{
  list_of_stats_.push_back(this);
}

//////////////////////////////////////////////////////////////////////////////

// Members

list<ac_stats_base*> ac_stats_base::list_of_stats_;

//////////////////////////////////////////////////////////////////////////////

// Methods

void ac_stats_base::print_all_stats(ostream& os)
{
  list<ac_stats_base*>::iterator it;
  for (it = list_of_stats_.begin(); it != list_of_stats_.end(); it++) {
    (*it)->print_stats(os);
  }
}

//////////////////////////////////////////////////////////////////////////////

// Destructors

/// ac_stats_base default destructor.
ac_stats_base::~ac_stats_base() {
}

//////////////////////////////////////////////////////////////////////////////


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


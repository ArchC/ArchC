/**
 * @file      ac_processor_stats.H
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
 * @date      Thu, 04 May 2006 14:24:27 -0300 
 *
 * @brief     Defines a template class for ArchC processor module statistics.
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

#ifndef AC_PROCESSOR_STATS_H
#define AC_PROCESSOR_STATS_H

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>
#include <iostream>

// SystemC includes

// ArchC includes
#include "ac_printable_stats.H"
#include "ac_stats_base.H"
#include "ac_basic_stats.H"

//////////////////////////////////////////////////////////////////////////////

// using statements
using std::string;
using std::ostream;
using std::endl;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

// Class declarations

/// Template class containing processor statistics.
template <class EN>
class ac_processor_stats : public ac_basic_stats<EN>, public ac_stats_base {
  private:
    using ac_basic_stats<EN>::number_of_stats_;
    using ac_basic_stats<EN>::stat_;
    using ac_basic_stats<EN>::stat_name_;

    string proc_name_;
    list<ac_printable_stats*> list_of_instr_stats_;

  public:
    /// Default constructor.
    explicit ac_processor_stats(const char* nm);

    /// Unhiding ac_basic_stats access operator.
    using ac_basic_stats<EN>::operator [];

    /// Printing method from ac_stats_base.
    void print_stats(ostream& os);

    /// Method that adds an ac_instruction_stats to the corresponding list.
    void add_instr_stats(ac_printable_stats* is);
};

//////////////////////////////////////////////////////////////////////////////

// Method definitions.

// ac_processor_stats<> methods.
template <class EN>
ac_processor_stats<EN>::ac_processor_stats(const char* nm) :
  ac_stats_base(),
  ac_basic_stats<EN>(),
  proc_name_(nm)
{}

template <class EN>
void ac_processor_stats<EN>::print_stats(ostream& os)
{
  os << "[ArchC 2.1] Printing GLOBAL statistics from processor module "
    << proc_name_ << ":" << endl;

  for (int i = 0; i < number_of_stats_; i++) {
    os << "     " << stat_name_[i] << " : " << stat_[i] << endl;
  }

  os << "[ArchC 2.1] Printing INSTRUCTION statistics from processor module "
    << proc_name_ << ":" << endl;

  list<ac_printable_stats*>::iterator it;
  for (it = list_of_instr_stats_.begin();
      it != list_of_instr_stats_.end();
      it++) {
    (*it)->print_stats(os);
  }
}

template <class EN>
void ac_processor_stats<EN>::add_instr_stats(ac_printable_stats* is)
{
  list_of_instr_stats_.push_back(is);
}

//////////////////////////////////////////////////////////////////////////////

#endif // AC_PROCESSOR_STATS_H


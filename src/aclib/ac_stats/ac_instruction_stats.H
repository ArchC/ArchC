/**
 * @file      ac_instruction_stats.H
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
 * @brief     Defines a template class for ArchC instruction statistics.
 *
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef AC_INSTRUCTION_STATS_H
#define AC_INSTRUCTION_STATS_H

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>
#include <iostream>

// SystemC includes

// ArchC includes
#include "ac_printable_stats.H"
#include "ac_basic_stats.H"
#include "ac_processor_stats.H"

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
class ac_instruction_stats : public ac_basic_stats<EN>,
			     public ac_printable_stats {
  private:
    using ac_basic_stats<EN>::number_of_stats_;
    using ac_basic_stats<EN>::stat_;
    using ac_basic_stats<EN>::stat_name_;

    string instr_name_;

  public:
    /// Default constructor.
    template <class P_EN>
    explicit ac_instruction_stats(const char* nm,
	ac_processor_stats<P_EN>& ps);

    /// Unhiding ac_basic_stats access operator.
    using ac_basic_stats<EN>::operator [];

    /// Printing method from ac_printable_stats.
    void print_stats(ostream& os);
};

//////////////////////////////////////////////////////////////////////////////

// Method definitions.

// ac_instruction_stats<> methods.
template <class EN>
template <class P_EN>
ac_instruction_stats<EN>::ac_instruction_stats(const char* nm,
    ac_processor_stats<P_EN>& ps) :
  ac_basic_stats<EN>(),
  instr_name_(nm)
{
  ps.add_instr_stats(this);
}

template <class EN>
void ac_instruction_stats<EN>::print_stats(ostream& os)
{
  os << "[ArchC 2.1] Printing statistics from instruction "
    << instr_name_ << ":" << endl;

  for (int i = 0; i < number_of_stats_; i++) {
    os << "     " << stat_name_[i] << " : " << stat_[i] << endl;
  }
}

//////////////////////////////////////////////////////////////////////////////

#endif // AC_INSTRUCTION_STATS_H


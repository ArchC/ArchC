/**
 * @file      ac_basic_stats.H
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
 * @date      Wed, 03 May 2006 16:00:36 -0300
 *
 * @brief     Defines a template class for ArchC statistics.
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

#ifndef AC_BASIC_STATS_H
#define AC_BASIC_STATS_H

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <string>

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// using statements
using std::string;

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

// Class declarations

/// Template class containing processor statistics.
template <class EN>
class ac_basic_stats {
  protected:
    static const int number_of_stats_ = EN::END_OF_STATS;
    long long stat_[number_of_stats_];
    //string proc_name_;
    string stat_name_[number_of_stats_];

  public:
    /// Default constructor.
    ac_basic_stats();

    /// Stats access operator.
    inline long long& operator [] (int which_stat);
};

//////////////////////////////////////////////////////////////////////////////

// Method definitions.

// ac_basic_stats<> methods.
template <class EN>
ac_basic_stats<EN>::ac_basic_stats()
{
  for (int i = 0; i < number_of_stats_; i++)
    stat_[i] = 0LL;
  
  string temp_names(EN::statnames);
  int current_stat_name = 0;
  string::size_type idx = 0;
  string::size_type last_idx = 0;
  do {
    last_idx = idx;
    idx = temp_names.find(',', last_idx);

    stat_name_[current_stat_name++] =
      temp_names.substr(last_idx, idx - last_idx);
    idx = temp_names.find_first_not_of(",\t\n ", idx);
  } while ((idx != string::npos) && (current_stat_name < number_of_stats_));
}

template <class EN>
long long& ac_basic_stats<EN>::operator [] (int which_stat)
{
  return stat_[which_stat];
}

//////////////////////////////////////////////////////////////////////////////

#endif // AC_BASIC_STATS_H


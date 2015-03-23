/**
 * @file      ac_instr.H
 * @author    Sandro Rigo
 *            Thiago Massariolli Sigrist
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Thu May 19 11:26:09 2005
 * 
 * @brief     Defines a container for instruction signals.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
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

#ifndef _AC_INSTR_H_
#define _AC_INSTR_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <iostream>

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// 'using' statements

using std::ostream;
using std::endl;

//////////////////////////////////////////////////////////////////////////////

/// ArchC class for instruction signals used in behavioral simulation.
template <int AC_DEC_FIELD_NUMBER> class ac_instr {

  /// Instruction fields container.
  unsigned instr[AC_DEC_FIELD_NUMBER];

public:

  /// Constructor.
  ac_instr( unsigned *im_instr ){

    if( im_instr ) {
      for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
        instr[j] = im_instr[j];
      }
    }
    else {
      for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
        instr[j] = 0;
      }
    }
  };
    
  /// Constructor. Fields set to zero.
  ac_instr(){

    for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
      instr[j] = 0;
    }
  };

  /// Attribution operator.
  ac_instr & operator= ( const ac_instr<AC_DEC_FIELD_NUMBER> &i){
    
    for( int j =0; j<AC_DEC_FIELD_NUMBER; j++){
      instr[j] = i.instr[j];
    }
    return *this;
  };
  
  /// Equality operator.
  friend bool operator== (const ac_instr<AC_DEC_FIELD_NUMBER> & i1,
                          const ac_instr<AC_DEC_FIELD_NUMBER> & i2) {
    bool flag = true;
    
    for( int j =0; j<AC_DEC_FIELD_NUMBER; j++) {
      flag = flag && (i1.instr[j] == i2.instr[j]);
    }
    
    return flag;
  };
  
  /// Used by stream insertion operator.
  void print(ostream& os) const {
    for (int j =0; j < AC_DEC_FIELD_NUMBER; j++) {
      os << instr[j] << ",";
    }
    os << endl;
  };

  /// Stream insertion operator.
  friend ostream& operator<< (ostream &os,
                              const ac_instr<AC_DEC_FIELD_NUMBER> &i) {
    i.print(os);
    return os;
  };
  
  /// Used in trace functions.
  operator bool () const {return true;};

  /// Get field method.
  unsigned get( const int i ){ return instr[i];  };
 
  /// Put field method.
  void put(const unsigned data, const unsigned i ){ instr[i] = data;  };
};

//////////////////////////////////////////////////////////////////////////////

/// Struct used for decoded instructions cache.
template <int AC_DEC_FIELD_NUMBER> struct cache_item {

  bool valid;
  ac_instr<AC_DEC_FIELD_NUMBER>* instr_p;
};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_INSTR_H_

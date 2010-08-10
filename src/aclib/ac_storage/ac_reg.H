/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_reg.H
 * @author    Sandro Rigo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Tue Sep 13 17:16:51 2005
 *
 * @brief     The ArchC register class.
 *            This class contains  members and methods needed by
 *            register devices for behavioral simulation. 
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef _AC_REG_H
#define _AC_REG_H

#include <string>
#include <list>

#include <systemc.h>

#include "ac_log.H"

using std::string;
using std::list;

////////////////////////////////////////////////////
//!ArchC class specialized for modeling registers.//
////////////////////////////////////////////////////
template <typename T> class ac_reg {
protected:
  T Data;
  string Name;

  typedef change_log<T> chg_log;
  typedef list<chg_log > log_list;
#ifdef AC_UPDATE_LOG
  log_list changes;                 //!< Update log.
  fstream update_file;              //!< Update log binary file.
#endif
  
#ifdef AC_DELAY
  log_list delays;                  //!< Delayed update queue.
  double& time_step;
#endif // AC_DELAY

public:
  
  //!Reading contents. Overloaded Method.
  const T& read( ) const {

    return Data;
  }

  //!Writing to the register. Overloaded Method.
  void write( T datum ) { 

#ifdef AC_VERBOSE
    changes.push_back( chg_log(0, datum , sc_simulation_time()));
#endif
  
    Data = datum;
 
  }

  /// Default constructor
  ac_reg(string name, T value):
    Data(value), Name(name) {}

#ifdef AC_DELAY
  /// Constructor with delay parameter
  ac_reg(string name, T value, double& ts):
    Data(value), Name(name), time_step(ts) {}
#endif // AC_DELAY

#ifdef AC_DELAY
  //!Writing to an address. Overloaded Method.
  void write( T datum, unsigned time ) { 
    delays.push_back( chg_log( 0, datum, (time * time_step) + time_step + sc_simulation_time()));
  }

  //!Commiting delayed updates
  void commit_delays( double time ){
    typename log_list::iterator itor;
    
    itor = delays.begin();
    while( delays.size() && (itor->time <= time) ){
      write( itor->value );
      itor = delays.erase( itor);
    }
  }

  //!Delayed assignement
  ac_reg& operator =( chg_log data ){

    delays.push_back( data );
    return *this;
  }
#endif  //AC_DELAY

  //!Convertion operator
  operator const T& () const { 
    return read();
  }

  //   //!Convertion operator
  //   operator ac_word()  { 

  //     return read();  
  //   }

  //    //!Convertion operator
  //    operator int()  { 
                
  //            return read();  
  //    }

  //   //!Convertion operator
  //   operator unsigned int()  { 

  //     return read();  
  //   }


  ////////////////////////////
  // OPERATORS !!!          //
  ////////////////////////////



  //!Overloaded assign operators

  //ac_reg<T> = ac_reg
  ac_reg<T>& operator = ( const ac_reg<T> &r ){ 
    write( r.read() );
    return *this;
  }

  //ac_reg<T> = Type_T
  ac_reg<T> & operator = ( const T& d ){ 

    write( d );
    return *this;  
  }

  //!ac_reg<T> *= Type T
  ac_reg<T> & operator *= ( const T v ){ 
    write( read() * (int)v );
    return *this;
  }

  //!ac_reg<T> *= ac_reg
  ac_reg<T> & operator *= ( ac_reg<T> &r ){ 
    write( read() * r.read() );
    return *this;
  }

  //!ac_reg<T> /= Type T
  ac_reg<T> & operator /= ( const T d ){ 
    write( read() / d );
    return *this;
  }

  //!ac_reg<T> /= ac_reg
  ac_reg<T> & operator /= ( ac_reg<T> &r ){ 
    write( read() / r.read() );
    return *this;
  }

  //!ac_reg<T> %= Type T
  ac_reg<T> & operator %= ( const T d ){ 
    write( read() % d );
    return *this;
  }

  //!ac_reg<T> %= ac_reg
  ac_reg<T> & operator %= ( ac_reg<T> &r ){ 
    write( read() % r.read() );
    return *this;
  }

  //!ac_reg<T> += Type T
  ac_reg<T> & operator += ( const T& d ){ 
    write( read() + d );
    return *this;
  }

  //!ac_reg<T> += ac_reg
  ac_reg<T> & operator += ( ac_reg<T> &r ){ 
    write( read() + r.read() );
    return *this;
  }

  //!ac_reg<T> -= Type T
  ac_reg<T> & operator -= ( const T d ){ 
    write( read() - d );
    return *this;
  }

  //!ac_reg<T> -= ac_reg
  ac_reg<T> & operator -= ( ac_reg<T> &r ){ 
    write( read() - r.read() );
    return *this;
  }

  //!ac_reg<T> <<= Type T
  ac_reg<T> & operator <<= ( const T d ){ 
    write( read() << d );
    return *this;
  }

  //!ac_reg<T> <<= ac_reg
  ac_reg<T> & operator <<= ( ac_reg<T> &r ){ 
    write( read() << r.read() );
    return *this;
  }

  //!ac_reg<T> >>= Type T
  ac_reg<T> & operator >>= ( const T d ){ 
    write( read() >> d );
    return *this;
  }

  //!ac_reg<T> >>= ac_reg
  ac_reg<T> & operator >>= ( ac_reg<T> &r ){ 
    write( read() >> r.read() );
    return *this;
  }

  //!ac_reg<T> &= Type T
  ac_reg<T> & operator &= ( const T d ){ 
    write( read() & d );
    return *this;
  }

  //!ac_reg<T> &= ac_reg
  ac_reg<T> & operator &= ( ac_reg<T> &r ){ 
    write( read() & r.read() );
    return *this;
  }

  //!ac_reg<T> ^= Type T
  ac_reg<T> & operator ^= ( const T d ){ 
    write( read() ^ d );
    return *this;
  }

  //!ac_reg<T> ^= ac_reg
  ac_reg<T> & operator ^= ( ac_reg<T> &r ){ 
    write( read() ^ r.read() );
    return *this;
  }

  //!ac_reg<T> |= Type T
  ac_reg<T> & operator |= ( const T d ){ 
    write( read() | d );
    return *this;
  }

  //!ac_reg<T> |= ac_reg
  ac_reg<T> & operator |= ( ac_reg<T> &r ){ 
    write( read() | r.read() );
    return *this;
  }

  //!Overloaded logical operators

  //   //ac_reg<T> == ac_reg
  //   bool operator == ( ac_reg<T> &r  ){ 

  //     if( read() == r.read())
  //                    return true;  
  //            else
  //                    return false;
  //   }

  // //ac_reg<T> != ac_reg
  //   bool operator != ( ac_reg<T> &r  ){ 

  //     if( read() != r.read())
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //ac_reg<T> >= ac_reg
  //   bool operator >= ( ac_reg<T> &r  ){ 

  //     if( read() >= r.read())
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //ac_reg<T> <= ac_reg
  //   bool operator <= ( ac_reg<T> &r  ){ 

  //     if( read() <= r.read())
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //ac_reg<T> > ac_reg
  //   bool operator > ( ac_reg<T> &r  ){ 

  //     if( read() > r.read())
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //ac_reg<T> < ac_reg
  //   bool operator < ( ac_reg<T> &r  ){ 

  //     if( read() < r.read())
  //       return true;  
  //     else
  //       return false;
  //   }


  //   //ac_reg<T> == Type_T
  //   bool operator == ( const T &i  ){ 

  //     if( read() == i)
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //!ac_reg<T> == Type_T
  //   bool operator != ( const T &i  ){ 

  //     if( read() != i)
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //!ac_reg<T> == Type_T
  //   bool operator >= ( const T &i  ){ 

  //     if( read() >= i)
  //       return true;  
  //     else
  //       return false;
  //   }
  //   //!ac_reg<T> <= Type_T
  //   bool operator <= ( const T &i  ){ 

  //     if( read() <= i)
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //!ac_reg<T> > Type_T
  //   bool operator > ( const T &i  ){ 

  //     if( read() >= i)
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //!ac_reg<T> < Type_T
  //   bool operator < ( const T &i  ){ 

  //     if( read() <= i)
  //       return true;  
  //     else
  //       return false;
  //   }

  //   //!Overloaded and operator
  //   unsigned operator & ( const T& d ){ 

  //     return read() & d;  
  //   }

  //   //!Overloaded and operator
  //   unsigned operator & ( ac_reg<T> &r ){ 

  //     return read() & r.read();  
  //   }

  //   //!Overloaded or operator
  //   unsigned operator | ( const T& d ){ 

  //     return read() | d;  
  //   }

  //   //!Overloaded or operator
  //   unsigned operator | ( ac_reg<T> &r ){ 

  //     return read() | r.read();  
  //   }

  //   //!Overloaded xor operator
  //   unsigned operator ^ ( const T& d ){ 

  //     return read() ^ d;  
  //   }

  //   //!Overloaded xor operator
  //   unsigned operator ^ ( ac_reg<T> &r ){ 

  //     return read() ^ r.read();  
  //   }

};

#endif // AC_REG_H

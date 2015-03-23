/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_sync_reg.H
 * @author    Sandro Rigo
 *            Marilia Felipe Chiozo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     The ArchC synchronous register class.
 *            This class contains  members and methods needed by
 *            synchronous register devices for behavioral simulation.
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

#ifndef _AC_SYNC_REG_H_
#define _AC_SYNC_REG_H_

// Standard includes.
#include <string>
// SystemC includes.
#include <systemc.h>
// ArchC includes.
#include "ac_log.H"

////////////////////////////////////////////////////
//!ArchC class specialized for modeling registers.//
////////////////////////////////////////////////////

template<class T> class ac_sync_reg:
 public sc_prim_channel
{
 protected:
  char* name; // register name
  T* Data; // out buffer
  T* NewData; // in buffer
  bool en; // internal enable
  unsigned int size; // size of the register
  typedef change_log<T> chg_log;
  typedef std::list<chg_log> log_list;
#ifdef AC_UPDATE_LOG
  log_list changes;                 //!< Update log.
  fstream update_file;              //!< Update log binary file.
#endif

  //! Clock process.
  void update()
  {
   if (en)
   {
#ifdef AC_VERBOSE
    changes.push_back(change_log(0, *NewData, sc_simulation_time()));
#endif
    if (Data != NewData)
     delete Data;
    Data = NewData;
#if 0 // def AC_STATS
   this->ac_sim_stats.add_access(name);
#endif
   }
   else
    en = true;
   return;
  }

 public:
#ifdef AC_UPDATE_LOG
  //! Reset log lists.
  void reset_log()
  {
   changes.clear();
   return;
  }

  //! Dump storage device log.
  int change_dump(ostream& output)
  {
   log_list::iterator itor;

   if (changes.size())
   {
    output << endl << endl;
    output << "**************** ArchC Change log *****************\n";
    output << "* Device: "<< name << "\t\t" << endl;
    output << "***************************************************\n";
    output << "*        Address         Value          Time      *\n";
    output << "***************************************************\n";
    for (itor = changes.begin(); itor != changes.end(); itor++)  
     output << "*  " << *itor << "     *" << endl;
    output << "***************************************************\n";
   }
   return 0;
  }

  //! Save storage device log.
  void change_save()
  {
   log_list::iterator itor;

   if (changes.size())
   {
    for (itor = changes.begin(); itor != changes.end(); itor++)
     itor->save(update_file);
   }
   return;
  }
#endif

  inline const T& read() const
  {
#if 0 // def AC_STATS
   this->ac_sim_stats.add_access(name);
#endif
   return *Data;
  }

  inline const T& async_read() const
  {
   return *NewData;
  }

  inline void write(T datum)
  {
   if (Data != NewData)
    delete NewData;
   NewData = new T(datum);
   request_update();
   return;
  }

  inline void async_write(T datum)
  {
   if (en)
   {
#ifdef AC_VERBOSE
    changes.push_back(change_log(0, datum, sc_simulation_time()));
#endif
    delete Data;
    if (Data != NewData)
     delete NewData;
    Data = NewData = new T(datum);
#if 0 // def AC_STATS
   this->ac_sim_stats.add_access(name);
#endif
   }
   return;
  }

  inline void suspend()
  {
   en = false;
   request_update(); // This looks weird, but en must go up by the end of the cycle.
   return;
  }

  ac_sync_reg(const char* mname, T value = 0):
   sc_prim_channel(mname)
  {
   Data = NewData = new T(value);
   en = true;
   size = sizeof(T);
   name = new char[1 + strlen(mname)];
   strcpy(name, mname);
   return;
  }

  ac_sync_reg(T value = 0):
   sc_prim_channel(sc_gen_unique_name("ac_sync_reg_anonymous"))
  {
   Data = NewData = new T(value);
   en = true;
   size = sizeof(T);
   name = new char[22];
   strcpy(name, "ac_sync_reg_anonymous");
   return;
  }

  ac_sync_reg(const ac_sync_reg<T>& src):
   sc_prim_channel(std::string(std::string(src.name) + std::string("_copy")).c_str())
  {
   Data = NewData = new T(src.read());
   en = src.en;
   size = sizeof(T);
   name = new char[6 + strlen(src.name)];
   strcpy(name, src.name);
   strcat(name, "_copy");
   return;
  }

  //! Destructor.
  virtual ~ac_sync_reg()
  {
#ifdef AC_UPDATE_LOG
   update_file.close();
#endif
   delete Data;
   if (Data != NewData)
    delete NewData;
   delete[] name;
   return;
  }

  //! Method to provide the name of the register.
  const char* get_name()
  {
   return name;
  }

  //!Method to provide the size of the register.
  const unsigned int get_size()
  {
   return size;
  }

#ifdef AC_UPDATE_LOG
  //!Method to provide the change list.
  log_list* get_changes()
  {
   return &changes;
  }
#endif

  //! Conversion operator.
  operator const T&() const
  {
   return read();
  }

  ////////////////////////////
  // OPERATORS !!!          //
  ////////////////////////////

  //! Overloaded assign operators. Chain carefully, as these assignments are synchronous.

  //! ac_sync_reg<T> = ac_sync_reg<T>
  ac_sync_reg<T>& operator=(const ac_sync_reg<T>& r)
  {
   write(r.read());
   return *this;
  }

  //! ac_sync_reg<T> = ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator=(const ac_sync_reg<U>& r)
  {
   write(static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> = Type U
  template<class U> ac_sync_reg<T>& operator=(const U d)
  {
   write(static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> *= ac_sync_reg<T>
  ac_sync_reg<T>& operator*=(const ac_sync_reg<T>& r)
  {
   write(read() * r.read());
   return *this;
  }

  //! ac_sync_reg<T> *= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator*=(const ac_sync_reg<U>& r)
  {
   write(read() * static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> *= Type U
  template<class U> ac_sync_reg<T>& operator*=(const U v)
  {
   write(read() * static_cast<T>(v));
    return *this;
  }

  //! ac_sync_reg<T> /= ac_sync_reg<T>
  ac_sync_reg<T>& operator/=(const ac_sync_reg<T>& r)
  {
   write(read() / r.read());
   return *this;
  }

  //! ac_sync_reg<T> /= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator/=(const ac_sync_reg<U>& r)
  {
   write(read() / static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> /= Type U
  template<class U> ac_sync_reg<T>& operator/=(const U d)
  {
   write(read() / static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> %= ac_sync_reg<T>
  ac_sync_reg<T>& operator%=(const ac_sync_reg<T>& r)
  {
   write(read() % r.read());
   return *this;
  }

  //! ac_sync_reg<T> %= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator%=(const ac_sync_reg<U>& r)
  {
   write(read() % static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> %= Type U
  template<class U> ac_sync_reg<T>& operator%=(const U d)
  {
   write(read() % static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> += ac_sync_reg<T>
  ac_sync_reg<T>& operator+=(const ac_sync_reg<T>& r)
  {
   write(read() + r.read());
   return *this;
  }

  //! ac_sync_reg<T> += ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator+=(const ac_sync_reg<U>& r)
  {
   write(read() + static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> += Type U
  template<class U> ac_sync_reg<T>& operator+=(const U& d)
  {
   write(read() + static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> -= ac_sync_reg<T>
  ac_sync_reg<T>& operator-=(const ac_sync_reg<T>& r)
  {
   write(read() - r.read());
   return *this;
  }

  //! ac_sync_reg<T> -= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator-=(const ac_sync_reg<U>& r)
  {
   write(read() - static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> -= Type U
  template <class U> ac_sync_reg<T>& operator-=(const U d)
  {
   write(read() - static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> <<= ac_sync_reg<T>
  ac_sync_reg<T>& operator<<=(const ac_sync_reg<T>& r)
  {
   write(read() << r.read());
   return *this;
  }

  //! ac_sync_reg<T> <<= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator<<=(const ac_sync_reg<U>& r)
  {
   write(read() << static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> <<= Type U
  template<class U> ac_sync_reg<T> & operator<<=(const U d)
  {
   write(read() << static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> >>= ac_sync_reg<T>
  ac_sync_reg<T>& operator>>=(const ac_sync_reg<T>& r)
  {
   write(read() >> r.read());
   return *this;
  }

  //! ac_sync_reg<T> >>= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator>>=(const ac_sync_reg<U>& r)
  {
   write(read() >> static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> >>= Type U
  template<class U> ac_sync_reg<T>& operator>>=(const U d)
  {
   write(read() >> static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> &= ac_sync_reg<T>
  ac_sync_reg<T>& operator&=(const ac_sync_reg<T>& r)
  {
   write(read() & r.read());
   return *this;
  }

  //! ac_sync_reg<T> &= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator&=(const ac_sync_reg<U>& r)
  {
   write(read() & static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> &= Type U
  template<class U> ac_sync_reg<T>& operator&=(const U d)
  {
   write(read() & static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> ^= ac_sync_reg<T>
  ac_sync_reg<T>& operator^=(const ac_sync_reg<T>& r)
  {
   write(read() ^ r.read());
   return *this;
  }

  //! ac_sync_reg<T> ^= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator^=(const ac_sync_reg<U>& r)
  {
   write(read() ^ static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> ^= Type U
  template<class U> ac_sync_reg<T>& operator^=(const U d)
  {
   write(read() ^ static_cast<T>(d));
   return *this;
  }

  //! ac_sync_reg<T> |= ac_sync_reg<T>
  ac_sync_reg<T>& operator|=(const ac_sync_reg<T>& r)
  {
   write(read() | r.read());
   return *this;
  }

  //! ac_sync_reg<T> |= ac_sync_reg<U>
  template<class U> ac_sync_reg<T>& operator|=(const ac_sync_reg<U>& r)
  {
   write(read() | static_cast<T>(r.read()));
   return *this;
  }

  //! ac_sync_reg<T> |= Type U
  template <class U> ac_sync_reg<T>& operator|=(const U d)
  {
   write(read() | static_cast<T>(d));
   return *this;
  }

  //! Overloaded logical operators.
  //! ac_sync_reg<T> == ac_sync_reg<T>
  bool operator==(const ac_sync_reg<T>& r) const
  {
   return (read() == r.read());
  }

  //! ac_sync_reg<T> == ac_sync_reg<U>
  template<class U> bool operator==(const ac_sync_reg<U>& r) const
  {
   return (read() == static_cast<T>(r.read()));
  }

  //! ac_sync_reg<T> == Type U
  template <class U> bool operator==(const U i) const
  {
   return (read() == static_cast<T>(i));
  }

  //! ac_sync_reg<T> != ac_sync_reg<T>
  bool operator!=(ac_sync_reg<T>& r) const
  {
   return (read() != r.read());
  }

  //! ac_sync_reg<T> != ac_sync_reg<U>
  template<class U> bool operator!=(const ac_sync_reg<U>& r) const
  {
   return (read() != static_cast<T>(r.read()));
  }

  //! ac_sync_reg<T> != Type U
  template <class U> bool operator!=(const U i) const
  {
   return (read() != static_cast<T>(i));
  }

  //! ac_sync_reg<T> >= ac_sync_reg<T>
  bool operator>=(ac_sync_reg<T>& r) const
  {
   return (read() >= r.read());
  }

  //! ac_sync_reg<T> >= ac_sync_reg<U>
  template<class U> bool
   operator>=(const ac_sync_reg<U>& r) const
  {
   return (read() >= static_cast<T>(r.read()));
  }

  //! ac_sync_reg<T> >= Type U
  template <class U> bool operator>=(const U i) const
  {
   return (read() >= static_cast<T>(i));
  }

  //! ac_sync_reg<T> <= ac_sync_reg<T>
  bool operator<=(ac_sync_reg<T>& r) const
  {
   return (read() <= r.read());
  }

  //! ac_sync_reg<T> <= ac_sync_reg<U>
  template<class U> bool operator<=(const ac_sync_reg<U>& r) const
  {
   return (read() <= static_cast<T>(r.read()));
  }

  //! ac_sync_reg<T> <= Type U
  template<class U> bool operator<=(const U i) const
  {
   return (read() <= static_cast<T>(i));
  }

  //! ac_sync_reg<T> > ac_sync_reg<T>
  bool operator>(ac_sync_reg<T>& r) const
  {
   return (read() > r.read());
  }

  //! ac_sync_reg<T> > ac_sync_reg<U>
  template<class U> bool operator>(const ac_sync_reg<U>& r) const
  {
   return (read() > static_cast<T>(r.read()));
  }

  //! ac_sync_reg<T> > Type U
  template<class U> bool operator>(const U i) const
  {
   return (read() > static_cast<T>(i));
  }

  //! ac_sync_reg<T> < ac_sync_reg<T>
  bool operator<(ac_sync_reg<T>& r) const
  {
   return (read() < r.read());
  }

  //! ac_sync_reg<T> < ac_sync_reg<U>
  template<class U> bool operator<(const ac_sync_reg<U>& r) const
  {
   return (read() < static_cast<T>(r.read()));
  }

  //! ac_sync_reg<T> < Type U
  template<class U> bool operator<(const U i) const
  {
   return (read() < static_cast<T>(i));
  }

#if 0
  //! Overloaded AND operator.
  T operator&(ac_sync_reg<T>& r) const
  {
   return (read() & r.read());
  }

  //! Overloaded AND operator.
  template<class U> T operator&(const ac_sync_reg<U>& r) const
  {
   return (read() & static_cast<T>(r.read()));
  }

  //! Overloaded AND operator.
  template<class U> T operator&(const U& d) const
  {
   return (read() & static_cast<T>(d));
  }

  //! Overloaded OR operator.
  T operator|(ac_sync_reg<T>& r) const
  {
   return (read() | r.read());
  }

  //! Overloaded OR operator.
  template<class U> T operator|(const ac_sync_reg<U>& r) const
  {
   return (read() | static_cast<T>(r.read()));
  }

  //! Overloaded OR operator.
  template<class U> T operator|(const U& d) const
  {
   return (read() | static_cast<T>(d));
  }

  //! Overloaded XOR operator.
  T operator^(ac_sync_reg<T>& r) const
  {
   return (read() ^ r.read());
  }

  //! Overloaded XOR operator.
  template<class U> T operator^(const ac_sync_reg<U>& r) const
  {
   return (read() ^ static_cast<T>(r.read()));
  }

  //! Overloaded XOR operator.
  template<class U> T operator^(const U& d) const
  {
   return (read() ^ static_cast<T>(d));
  }
#endif
};

#endif // AC_SYNC_REG_H

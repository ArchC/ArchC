/**
 * @file      ac_stage.H
 * @author    Marilia Felippe Chiozo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   version?
 * @date      Mon, 30 May 2005 06:04:52 -0200
 * 
 * @brief     brief?
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

#ifndef _AC_STAGE_H_
#define _AC_STAGE_H_

// Standard includes.
#include <string.h>
// SystemC includes.
#include <systemc.h>
// ArchC includes.
#include "ac_sync_reg.H"
#include "ac_instr.H"

//! ArchC base class for pipeline stages.
template<class ac_instr> class ac_stage
{
 protected:
  class ac_stage_control: public sc_prim_channel
  {
   public:
    /// Flush control variable.
    bool fctrl_next;
    /// Stall control variables.
    bool sctrl_next;
    bool sctrl_now;

    /// Constructor.
    ac_stage_control()
    {
     fctrl_next = sctrl_next = sctrl_now = false;
     return;
    }

    void update()
    {
     fctrl_next = false; // Flushing already took place (or will before anyone can ask).
     sctrl_now = sctrl_next; // Stall state copied from future stall state.
     sctrl_next = false; // Future stall state defaults to false.
     return;
    }
  };

  ac_stage_control ctrl;
  char* name;
  ac_stage<ac_instr>* prev;
  ac_sync_reg<ac_instr>* regin;
  ac_sync_reg<ac_instr>* regout;

 public:
  virtual void behavior() = 0;

  void flush()
  {
   static ac_instr* nop = new ac_instr;

   ctrl.fctrl_next = true;
   if (regout)
    regout->write(*nop);
   ctrl.request_update();
   return;
  }

  bool will_flush()
  {
   return ctrl.fctrl_next;
  }

  void stall()
  {
   ac_stage<ac_instr>* p;
   static ac_instr* nop = new ac_instr;

   ctrl.sctrl_next = true;
   if (regout)
    regout->write(*nop);
   p = this;
   while (p->prev)
   {
    p->regin->suspend();
    p = p->prev;
   }
   ctrl.request_update();
   return;
  }

  bool is_stalled()
  {
   return ctrl.sctrl_now;
  }

  bool will_stall()
  {
   return ctrl.sctrl_next;
  }

  const char* get_name() const
  {
   return name;
  }

  ac_stage(const char* _name, ac_stage<ac_instr>* p, ac_sync_reg<ac_instr>* ri,
           ac_sync_reg<ac_instr>* ro):
   prev(p),
   regin(ri),
   regout(ro)
  {
   name = new char[1 + strlen(_name)];
   strcpy(name, _name);
   return;
  }

  virtual ~ac_stage()
  {
   return;
  }
};

#endif // _AC_STAGE_H_

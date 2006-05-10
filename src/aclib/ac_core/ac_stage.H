/***************************************************************************
 *            ac_stage.H
 *
 *  Mon May 30 06:04:52 2005
 *  Copyright 2005  Marilia Felippe Chiozo
 *  Email ra030038@ic.unicamp.br
 ****************************************************************************/

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

template<class ac_instr> class ac_stage: public sc_prim_channel
{
 private:
  /// Flush control variable.
  bool fctrl_next;
  /// Stall control variables.
  bool sctrl_next;
  bool sctrl_now;
 protected:
  char* name;
  ac_stage<ac_instr>* prev;
  ac_sync_reg<ac_instr>* regin;
  ac_sync_reg<ac_instr>* regout;

  virtual void update()
  {
   fctrl_next = false; // Flushing already took place (or will before anyone can ask).
   sctrl_now = sctrl_next; // Stall state copied from future stall state.
   sctrl_next = false; // Future stall state defaults to false.
   return;
  }

 public:
  virtual void behavior() = 0;

  void flush()
  {
   static ac_instr* nop = new ac_instr;

   fctrl_next = true;
   if (regout)
    regout->write(*nop);
   request_update();
   return;
  }

  bool will_flush()
  {
   return fctrl_next;
  }

  void stall()
  {
   ac_stage<ac_instr>* p;
   static ac_instr* nop = new ac_instr;

   sctrl_next = true;
   if (regout)
    regout->write(*nop);
   p = this;
   while (p->prev)
   {
    p->regin->suspend();
    p = p->prev;
   }
   request_update();
   return;
  }

  bool is_stalled()
  {
   return sctrl_now;
  }

  bool will_stall()
  {
   return sctrl_next;
  }

  const char* get_name() const
  {
   return name;
  }

  ac_stage(const char* _name, ac_stage<ac_instr>* p, ac_sync_reg<ac_instr>* ri,
           ac_sync_reg<ac_instr>* ro):
   sc_prim_channel(_name),
   fctrl_next(false),
   sctrl_next(false),
   sctrl_now(false),
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

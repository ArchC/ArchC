/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* Initialize ___arch_name___ target dependent functions for GDB, the GNU debugger.
   Copyright 2005, 2006 --- The ArchC Team

This file is automatically retargeted by ArchC binutils generation tool. 
This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * written by:
 *   Alexandre Keunecke I. de Mendonca            
 *   Felipe Guimaraes Carvalho                    
 *   Max Ruben de Oliveira Schultz                
 */

#include "defs.h"
#include "frame.h"
#include "frame-base.h"
#include "trad-frame.h"
#include "frame-unwind.h"
#include "dwarf2-frame.h"
#include "gdbtypes.h"
#include "inferior.h"
#include "gdb_string.h"
#include "gdb_assert.h"
#include "gdbcore.h"
#include "arch-utils.h"
#include "regcache.h"
#include "dis-asm.h"
#include "osabi.h"
#include `"opcode/'___arch_name___`.h"'


___defines_gdb___


#define X_OP(i) (((i) >> 30) & 0x3)
#define X_OP3(i) (((i) >> 19) & 0x3f)

enum
{ 
  `'___arch_name___`_INSN32_SIZE = 4,'
};

/*---------------------------------------------------------------------------------------------------*/

struct `'___arch_name___`_frame_cache'
{
  CORE_ADDR base;
  struct trad_frame_saved_reg *saved_regs;
  int frameless_p;
};

/*---------------------------------------------------------------------------------------------------*/

static CORE_ADDR
`'___arch_name___`_skip_prologue (CORE_ADDR pc);'

static CORE_ADDR
`'___arch_name___`_scan_prologue (CORE_ADDR start_pc, CORE_ADDR limit_pc,'
  struct frame_info *next_frame,
  struct `'___arch_name___`_frame_cache *this_cache);'

static ULONGEST
`'___arch_name___`_fetch_instruction (CORE_ADDR addr);'

static const gdb_byte *
`'___arch_name___`_breakpoint_from_pc (CORE_ADDR *pcptr, int *lenptr);'

static CORE_ADDR 
read_next_frame_reg (struct frame_info *, int);

static void
set_reg_offset (struct `'___arch_name___`_frame_cache *this_cache, int regnum,' CORE_ADDR offset);

static struct type *
`'___arch_name___`_register_type (struct gdbarch *gdbarch, int reg_num);'

static CORE_ADDR
`'___arch_name___`_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame);'

static const struct frame_unwind *
`'___arch_name___`_frame_sniffer (struct frame_info *next_frame);'

static void
`'___arch_name___`_frame_this_id (struct frame_info *next_frame, void **this_cache,'
  struct frame_id *this_id);

static void
`'___arch_name___`_frame_prev_register (struct frame_info *next_frame, void **this_cache,'
  int regnum, int *optimizedp,
  enum lval_type *lvalp, CORE_ADDR *addrp,
  int *realnump, void *valuep);

static struct `'___arch_name___`_frame_cache *'
`'___arch_name___`_frame_cache (struct frame_info *next_frame, void **this_cache);'

static const char *
`'___arch_name___`_register_name (int reg_num);'

static struct frame_id
`'___arch_name___`_unwind_dummy_id (struct gdbarch *gdbarch, struct frame_info *next_frame);'

static const struct frame_base *
`'___arch_name___`_frame_base_sniffer (struct frame_info *next_frame);'

static CORE_ADDR
`'___arch_name___`_frame_base_address (struct frame_info *next_frame, void **this_cache);'

/*---------------------------------------------------------------------------------------------------*/

/* To skip prologues, I use this predicate.  Returns either PC itself
   if the code at PC does not look like a function prologue; otherwise
   returns an address that (if we're lucky) follows the prologue.  If
   LENIENT, then we must skip everything which is involved in setting
   up the frame (it's OK to skip more, just so long as we don't skip
   anything which might clobber the registers which are being saved.
   We must skip more in the case where part of the prologue is in the
   delay slot of a non-prologue instruction).  */
static CORE_ADDR
`'___arch_name___`_skip_prologue (CORE_ADDR pc)'
{  
  CORE_ADDR limit_pc;
  CORE_ADDR func_addr;

  /* See if we can determine the end of the prologue via the symbol table.
     If so, then return either PC, or the PC after the prologue, whichever is greater.  */
  if (find_pc_partial_function (pc, NULL, &func_addr, NULL)) {
    CORE_ADDR post_prologue_pc = skip_prologue_using_sal (func_addr);
    if (post_prologue_pc != 0) {
      return max (pc, post_prologue_pc);
    } 
  }
}

/* Fetch and return instruction from the specified location.  */
static ULONGEST
`'___arch_name___`_fetch_instruction (CORE_ADDR addr)'
{ 
  `gdb_byte buf['___arch_name___`_INSN32_SIZE];'
  int instlen;
  int status;

  instlen = `'___arch_name___`_INSN32_SIZE;'
  status = deprecated_read_memory_nobpt (addr, buf, instlen);
  if (status)
    memory_error (status, addr);
  return extract_unsigned_integer (buf, instlen);
}

/* This function implements the BREAKPOINT_FROM_PC macro.
   Use the program counter to determine the contents and size of a breakpoint instruction.
   It returns a pointer to a string of bytes that encode a breakpoint instruction, stores 
   the length of the string to *lenptr, and adjusts pc (if necessary) to point to the actual 
   memory location where the breakpoint should be inserted. */
static const gdb_byte * 
`'___arch_name___`_breakpoint_from_pc (CORE_ADDR *pcptr, int *lenptr)'
{
}

static CORE_ADDR
read_next_frame_reg (struct frame_info *fi, int regno)
{  
  if (fi == NULL) {
    LONGEST val;
    regcache_cooked_read_signed (current_regcache, regno, &val);
    return val;
  }
  else
    return frame_unwind_register_signed (fi, regno);
}

/* Set a register's saved stack address in temp_saved_regs.  If an
   address has already been set for this register, do nothing; this
   way we will only recognize the first save of a given register in a
   function prologue.

   For simplicity, save the address in both [0 .. NUM_REGS) and
   [NUM_REGS .. 2*NUM_REGS).  Strictly speaking, only the second range
   is used as it is only second range (the ABI instead of ISA
   registers) that comes into play when finding saved registers in a
   frame.  */
static void
set_reg_offset (struct `'___arch_name___`_frame_cache *this_cache, int regnum,' CORE_ADDR offset)
{ 
  if (this_cache != NULL && this_cache->saved_regs[regnum].addr == -1) {
    this_cache->saved_regs[regnum + 0 * NUM_REGS].addr = offset;
    this_cache->saved_regs[regnum + 1 * NUM_REGS].addr = offset;
  }
}

/* Return the GDB type object for the "standard" data type of data in register "regnum". */
static struct type *
`'___arch_name___`_register_type (struct gdbarch *gdbarch, int regnum)'
{
  return builtin_type_int32;
}

/* Given THIS_FRAME, find the previous frame's resume PC (which will be used 
   to construct the previous frame's ID, after looking up the containing function). */
static CORE_ADDR
`'___arch_name___`_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)'
{
  return frame_unwind_register_signed (next_frame, NUM_REGISTER_PC);
}

/* Given a GDB frame, determine the address of the calling function's frame. 
   This will be used to create a new GDB frame struct. */
static void
`'___arch_name___`_frame_this_id (struct frame_info *next_frame, void **this_cache,'
  struct frame_id *this_id)
{
  `struct '___arch_name___`_frame_cache *info = ' `'___arch_name___`_frame_cache (next_frame, this_cache);'
  (*this_id) = frame_id_build (info->base, frame_func_unwind (next_frame));
}

`static const struct frame_unwind '___arch_name___`_frame_unwind = {'
  NORMAL_FRAME,
  `'___arch_name___`_frame_this_id,'
  `'___arch_name___`_frame_prev_register'
};

static const struct frame_unwind *
`'___arch_name___`_frame_sniffer (struct frame_info *next_frame)'
{
  CORE_ADDR pc = frame_pc_unwind (next_frame);
  return `&'___arch_name___`_frame_unwind;'
}

/* Return the name of the register corresponding to "regnum". */
static const char *
`'___arch_name___`_register_name (int regnum)'
{
  acasm_symbol *symbols = (acasm_symbol *) udsymbols; 

  int cont = 0;
  int find_reg_name = 0;
  while (cont < num_symbols) {
    if (regnum == symbols->value) { //reg_name find
      find_reg_name = 1;
      break;
    }
    symbols++;
    cont++;
  }

  if (find_reg_name) {
    return symbols->symbol; 
  }
  else { 
    static char ret[5];
    sprintf(ret, "$%i\0", regnum); 
    return ret;
  }
}

static struct frame_id
`'___arch_name___`_unwind_dummy_id (struct gdbarch *gdbarch, struct frame_info *next_frame)'
{
  return frame_id_build (frame_unwind_register_signed (next_frame, NUM_REGISTER_SP),
			 frame_pc_unwind (next_frame)); 
}

static CORE_ADDR
`'___arch_name___`_frame_base_address (struct frame_info *next_frame, void **this_cache)'
{
  `struct '___arch_name___`_frame_cache *info = ' `'___arch_name___`_frame_cache (next_frame, this_cache);'
  return info->base;
}

static const struct frame_base `'___arch_name___`_frame_base ='
{
  `&'___arch_name___`_frame_unwind,'
  `'___arch_name___`_frame_base_address,'
  `'___arch_name___`_frame_base_address,'
  `'___arch_name___`_frame_base_address'
};

static const struct frame_base *
`'___arch_name___`_frame_base_sniffer (struct frame_info *next_frame)'
{
  if `('___arch_name___`_frame_sniffer (next_frame) != NULL)'
    return `&'___arch_name___`_frame_base;'
  else
    return NULL;
}

/* Put here the code to store, into fi->saved_regs, the addresses of
   the saved registers of frame described by FRAME_INFO.  This
   includes special registers such as pc and fp saved in special ways
   in the stack frame.  sp is even more special: the address we return
   for it IS the sp for the next frame. */
static struct `'___arch_name___`_frame_cache *'
`'___arch_name___`_frame_cache (struct frame_info *next_frame, void **this_cache)'
{
  struct `'___arch_name___`_frame_cache *cache;'

  if ((*this_cache) != NULL)
    return (*this_cache);

  cache = FRAME_OBSTACK_ZALLOC (struct `'___arch_name___`_frame_cache);'
  cache->frameless_p=1;
  (*this_cache) = cache;
  cache->saved_regs = trad_frame_alloc_saved_regs (next_frame);

  /* Analyze the function prologue.  */  
  const CORE_ADDR pc = frame_pc_unwind (next_frame);
  CORE_ADDR start_addr;

  find_pc_partial_function (pc, NULL, &start_addr, NULL);    
  /* We can't analyze the prologue if we couldn't find the begining of the function.  */
  if (start_addr == 0)
    return cache;  

  `'___arch_name___`_scan_prologue (start_addr, pc, next_frame, *this_cache);'

  if (cache->frameless_p)
    cache->base = frame_unwind_register_unsigned (next_frame, NUM_REGISTER_SP);
  else
    cache->base = frame_unwind_register_unsigned (next_frame, NUM_REGISTER_FP);

  return (*this_cache);
}


static void
`'___arch_name___`_frame_prev_register (struct frame_info *next_frame, void **this_cache,'
  int regnum, int *optimizedp,
  enum lval_type *lvalp, CORE_ADDR *addrp,
  int *realnump, void *valuep)
{
  `struct '___arch_name___`_frame_cache *info =' `'___arch_name___`_frame_cache (next_frame, this_cache);'

  if (regnum == NUM_REGISTER_PC) {
    if (valuep) {
      CORE_ADDR pc = 0;
      regnum = info->frameless_p ? NUM_REGISTER_RET1 : NUM_REGISTER_RET2;
      pc += frame_unwind_register_unsigned (next_frame, regnum) + OFFSET + 4;
      store_unsigned_integer (valuep, 4, pc);
    }
   if (NUM_REGISTER_RET2 != -1)
     return; //only sparc
  }

  trad_frame_get_prev_register (next_frame, info->saved_regs, regnum, optimizedp, lvalp, addrp, realnump, valuep);
}

/* Analyze the function prologue from START_PC to LIMIT_PC.
   Builds the associated FRAME_CACHE if not null.
   Return the address of the first instruction past the prologue.  */
static CORE_ADDR
`'___arch_name___`_scan_prologue (CORE_ADDR start_pc, CORE_ADDR limit_pc,'
  struct frame_info *next_frame,
  struct `'___arch_name___`_frame_cache *this_cache)'
{
  CORE_ADDR cur_pc;
  CORE_ADDR sp;

  /* Can be called when there's no process, and hence when there's no NEXT_FRAME.  */
  if (next_frame != NULL)
    sp = read_next_frame_reg (next_frame, NUM_REGISTER_SP);
  else
    sp = 0;

  if (start_pc < limit_pc) {
    unsigned long inst;

    /* Fetch the instruction.   */
    inst = (unsigned long) `'___arch_name___`_fetch_instruction (start_pc);'

    /* Check for the SAVE instruction that sets up the frame, only sparc.  */
    if ((NUM_REGISTER_RET2 != -1) && (X_OP (inst) == 2 && X_OP3 (inst) == 0x3c))
      this_cache->frameless_p = 0;
    
    set_reg_offset (this_cache, NUM_REGISTER_RET1, limit_pc+OFFSET+4);
  }

  if (this_cache != NULL) {
    this_cache->base = (frame_unwind_register_signed (next_frame, NUM_REGISTER_SP));
    this_cache->saved_regs[NUM_REGISTER_PC] = this_cache->saved_regs[NUM_REGISTER_RET1];
  }
}

/* linking between gdb core functions and local functions */
static struct gdbarch *
`'___arch_name___`_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)'
{
  struct gdbarch *gdbarch;

  if (info.bfd_arch_info->arch != `bfd_arch_'___arch_name___`)'
    return NULL;

  gdbarch = gdbarch_alloc (&info, NULL);

  /* Information about registers */
  set_gdbarch_num_regs (gdbarch, NUM_REGISTERS);
  set_gdbarch_sp_regnum (gdbarch, NUM_REGISTER_SP);
  set_gdbarch_pc_regnum (gdbarch, NUM_REGISTER_PC);
  set_gdbarch_register_name (gdbarch, `'___arch_name___`_register_name);'
  set_gdbarch_register_type (gdbarch, `'___arch_name___`_register_type);'
  set_gdbarch_num_pseudo_regs (gdbarch, 0);

  /* The stack grows downward. */
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);

  /* Advance PC across function entry code. */
  set_gdbarch_skip_prologue (gdbarch, `'___arch_name___`_skip_prologue);'

  /* Breakpoint manipulation. */
  set_gdbarch_breakpoint_from_pc (gdbarch, `'___arch_name___`_breakpoint_from_pc);'

  /* Disassembly. */
  set_gdbarch_print_insn (gdbarch, print_insn_`'___arch_name___`);'

  /* Add some default predicates. */
  frame_unwind_append_sniffer (gdbarch, `'___arch_name___`_frame_sniffer);'
  frame_base_append_sniffer (gdbarch, `'___arch_name___`_frame_base_sniffer);'

  /* Frame handling. */
  set_gdbarch_unwind_pc (gdbarch, `'___arch_name___`_unwind_pc);'
  set_gdbarch_unwind_dummy_id (gdbarch, `'___arch_name___`_unwind_dummy_id);'

  set_gdbarch_short_bit (gdbarch, 2 * TARGET_CHAR_BIT);
  set_gdbarch_int_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_long_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_long_long_bit (gdbarch, 8 * TARGET_CHAR_BIT);
  set_gdbarch_float_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_double_bit (gdbarch, 8 * TARGET_CHAR_BIT);
  set_gdbarch_long_double_bit (gdbarch, 8 * TARGET_CHAR_BIT);
  set_gdbarch_ptr_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_addr_bit (gdbarch, 4 * TARGET_CHAR_BIT);

  /* Hook in ABI-specific overrides, if they have been registered. */
  gdbarch_init_osabi (info, gdbarch);  

  return gdbarch;
}

extern `initialize_file_ftype _initialize_'___arch_name___`_tdep;' /* -Wmissing-prototypes */

/* Start of target-dependent code for the architecture */
void 
`_initialize_'___arch_name___`_tdep (void)'
{
  register_gdbarch_init (`bfd_arch_'___arch_name___`,' `'___arch_name___`_gdbarch_init);'
}

/*---------------------------------------------------------------------------------------------------*/

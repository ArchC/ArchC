/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      utils.h
 * @author    Alexandro Baldassin (UNICAMP)
 *            Daniel Casarotto (UFSC)
 *            Max Schultz (UFSC)
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 *            System Design Automation Lab (LAPS)
 *            INE-UFSC
 *            http://www.laps.inf.ufsc.br/
 * 
 * @version   1.0
 * @date      Thu, 01 Jun 2006 14:28:06 -0300
 * 
 * @brief     Utilities routines for the acbinutils module
 * 
 * This module implements common routines shared by the 
 * binary utilities generation files.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/** @defgroup utils_group Utilities routines
 * @ingroup binutils_group gas_group bfd_group opcodes_group
 *  
 * @{
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include "acpp.h"

/* Indentation */
#define IND1 "  "
#define IND2 "    "
#define IND3 "      "
#define IND4 "        "
#define IND5 "          "

extern unsigned int encode_fields(ac_asm_insn_field *fields);
extern int get_format_id(char *fname);
extern unsigned int get_arch_size();
extern unsigned int get_insn_size(ac_asm_insn *insn);
extern unsigned int get_max_format_size();
extern unsigned int get_variable_format_size();
extern unsigned int get_format_number_fields(ac_dec_format *format);
extern unsigned int get_max_number_fields();
extern void set_arch_name(char *str);
extern char *get_arch_name();
extern void internal_error();

extern int log_table[];

/* got from architecture file by the parser */
extern char *isa_filename;  
extern int ac_tgt_endian; /* filled by the parser 0=l, 1=b */

extern ac_dec_format *format_ins_list;


#endif /* _UTILS_H_ */

/* @} */


/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      core_actions.h
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Alexandro Baldassin
 *            Thiago Sigrist
 *            Marilia Chiozo
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Fri, 02 Jun 2006 10:59:18 -0300
 *
 * @brief     Core language semantic actions
 *
 * This file contain the semantic actions and variables called/used
 * by the main parser module \ref bison_group
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

/** @defgroup coreact_group Core semantic actions
 * @ingroup bison_group
 *
 * Exported variables are filled by the parser and then can be used by
 * the generator tools. The interface functions are used by the parser
 * to deal with the data structures.
 * In the future each variable should be changed by an interface
 * function, hence avoiding global ones.
 *
 * @{
 */

#ifndef _CORE_ACTIONS_H_
#define _CORE_ACTIONS_H_

#include <stdio.h>
#include <string.h>
#include "ac_decoder.h"
#include "ac_tools_common.h"

/** Size of the acpp symbol table. */
#define ST_IDX_SIZE 9

/** Enumeration type for the symbol classes in the symbol table. */
typedef enum _ac_parser_type {INSTR, INSTR_FMT, REG_FMT, INSTR_GRP, GEN_ID} ac_parser_type;

/** Struct to hold symbol table information. */
typedef struct _symbol_table_entry
{
 char* name; //!< The name of the entry.
 ac_parser_type type; //!< The symbol class of the entry.
 void* info; //!< Pointer to the parser structure with information on the symbol.
 struct _symbol_table_entry* next; //!< Pointer to the next entry (for hash clashes).
} symbol_table_entry;

//extern symbol_table_entry* symbol_table[1 << ST_IDX_SIZE]; // I decided to make this local to core_actions.c. --Marilia
extern ac_dec_format* format_ins_list;    //!< Format List for instructions.
extern ac_dec_format* format_ins_list_tail;
extern ac_dec_field* common_instr_field_list;  //!< List containing all the fields common to all instructions.
extern ac_dec_field* common_instr_field_list_tail;
extern ac_dec_format* format_reg_list;    //!< Format List for registers.
extern ac_dec_format* format_reg_list_tail;
extern ac_dec_instr* instr_list;          //!< Instruction List.
extern ac_dec_instr* instr_list_tail;
extern ac_grp_list* group_list;           //!< List of instruction groups.
extern ac_grp_list* group_list_tail;
extern ac_pipe_list* pipe_list;           //!< Pipe list
extern ac_stg_list* stage_list;           //!< old 'ac_stages' list for pipe stages
extern ac_sto_list* storage_list;         //!< Storage list
extern ac_sto_list* storage_list_tail;
extern ac_sto_list* tlm_intr_port_list;   //!< List of TLM Interrupt ports
extern ac_sto_list* tlm_intr_port_list_tail;

/** Boolean flag passed to the SystemC simulator generator */
extern int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, HaveCycleRange;
extern int ControlInstrInfoLevel;
extern int HaveTLMPorts;
extern int HaveTLMIntrPorts;
extern int HaveTLM2Ports;
extern int HaveTLM2NBPorts;
extern int HaveTLM2IntrPorts;

extern int instr_num;    //!< Number of Instructions
extern int declist_num;  //!< Number of Decodification lists
extern int format_num;   //!< Number of Formats
extern int group_num;    //!< Number of Groups
extern int const_count;  //!< Number of Constants
extern int stage_num;    //!< Number of Stages
extern int pipe_num;     //!< Number of Pipelines
extern int reg_width;    //!< Bit width of registers in a regbank.
extern int largest_format_size;

extern ac_sto_list* fetch_device; //!< Indicates the device used for fetching instructions.

/* functions used in the semantic actions */
extern void init_core_actions();
extern symbol_table_entry* find_symbol(char* name, ac_parser_type type);
extern int add_symbol(char* name, ac_parser_type type, void* info);
extern ac_dec_instr* find_instr(char* name);
extern ac_dec_format* find_format(char* name);
extern ac_sto_list *find_storage(char* name);
extern ac_dec_field* find_field(ac_dec_format* pformat, char* name);
extern int add_format(ac_dec_format** head, ac_dec_format** tail, char* name, char* str, char* error_msg, int is_instr);
extern int add_instr(char* name, char *typestr, ac_dec_instr** pinstr, char* error_msg);
extern ac_grp_list* add_group(char* name);
extern int add_instr_ref(char* name, ac_instr_ref_list** instr_refs, char* error_msg);
extern ac_pipe_list* add_pipe(char* name);
extern ac_stg_list* add_stage(char* name, ac_stg_list** listp);
extern int add_storage(char* name, unsigned size, ac_sto_types type, char* typestr, char* error_msg);
extern int add_dec_list(ac_dec_instr* pinstr, char* name, int value, char* error_msg);
extern ac_control_flow* get_control_flow_struct(ac_dec_instr* pinstr);
extern void add_parms(char* name, int value);
extern void str_upper(char* str);

/*@}*/

#endif /* _CORE_ACTIONS_H_ */

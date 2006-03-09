/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/

#ifndef _CORE_ACTIONS_H_
#define _CORE_ACTIONS_H_

#include "ac_decoder.h"
#include "acsim.h"

extern ac_dec_format *format_ins_list;    //! Format List for instructions.
extern ac_dec_format *format_ins_list_tail;
extern ac_dec_format *format_reg_list;    //!< Format List for registers.
extern ac_dec_format *format_reg_list_tail;
extern ac_dec_instr *instr_list;          //!< Instruction List.
extern ac_pipe_list *pipe_list;           //!< Pipe list
extern ac_stg_list  *stage_list;          //!< old 'ac_stages' list for pipe stages
extern ac_sto_list  *storage_list;        //!< Storage list

//@{
/** Boolean flag passed to the SystemC simulator generator */
extern int HaveFormattedRegs, HaveMultiCycleIns, HaveMemHier, HaveCycleRange;
extern int ControlInstrInfoLevel;
extern int HaveTLMPorts;
extern int HaveTLMIntrPorts;
//@}

/* TODO: Check if all of these variables must be global */
extern int instr_num;    //!< Number of Instructions 
extern int declist_num;  //!< Number of Decodification lists
extern int format_num;   //!< Number of Formats
extern int const_count;  //!< Number of Constants
extern int stage_num;    //!< Number of Stages
extern int pipe_num;     //!< Number of Pipelines
extern int reg_width;   //!< Bit width of registers in a regbank.
extern int largest_format_size;

extern ac_sto_list *fetch_device; //!< Indicates the device used for fetching instructions.


/* functions used in the semantic actions */
extern void init_core_actions();
extern ac_dec_instr *find_instr(char *name);
extern ac_dec_format *find_format(char *name);
extern ac_sto_list *find_storage(char *name);
extern ac_dec_field *find_field(ac_dec_format *pformat, char *name);   
extern int add_format( ac_dec_format **head, ac_dec_format **tail, char *name, char* str, char *error_msg);
extern int add_instr(char* name, char *typestr, ac_dec_instr **pinstr, char *error_msg);
extern ac_pipe_list *add_pipe(char* name);
extern void add_stage( char* name, ac_stg_list** listp );
extern int add_storage( char* name, unsigned size, ac_sto_types type, char *typestr, char *error_msg);
extern int add_dec_list( ac_dec_instr *pinstr, char* name, int value, char *error_msg);
extern ac_control_flow *get_control_flow_struct(ac_dec_instr *pinstr);
extern void add_parms(char *name, int value);
   
#endif /* _CORE_ACTIONS_H_ */

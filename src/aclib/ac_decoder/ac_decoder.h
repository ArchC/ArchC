/* ex: set tabstop=2 expandtab:   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*- */

/**
 * @file      ac_decoder.h
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Wed, 07 Jun 2006 16:48:24 -0300
 *
 * @brief     ArchC Decoder header file.
 *            This file contains the structures needed for the ArchC
 *            Pre-processor to build the decoder for the target ISA.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef _AC_DECODER_H_
#define _AC_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif


//! Field type used to build the decoder
typedef struct _ac_dec_field{
  char *name;                 //!< Field name
  int size;                   //!< Field size in bits
  int first_bit;              //!< First bit of the field inside the instruction
  int id;                     //!< Unique id that identifies this field
  long val;                   //!< Value of the field
  int sign;                   //!< Indicates whether the field is signed or not
  struct _ac_dec_field *next; //!< Next field
} ac_dec_field;

//! Format type used to build the decoder
typedef struct _ac_dec_format{
  char *name;                  //!< Format name
  int size;                    //!< Format size in bits
  ac_dec_field *fields;        //!< List of fields in this format
  struct _ac_dec_format *next; //!< Next format
} ac_dec_format;

//! Type used to build the decoder
typedef struct _ac_dec_list{
  char *name;                   //!< Field name to be checked
  int id;                       //!< Field ID to be checked
  int value;                    //!< Value to find
  struct _ac_dec_list *next; //!< Next decode element
} ac_dec_list;

//! Type used for control flow instructions
typedef struct _control_flow {
  char *cond;                 //!< Condition for control instruction
  char *target;               //!< Target for the jump/branch
  int  delay_slot;            //!< Number of delay slots
  char *delay_slot_cond;      //!< Condition to execute instructions in the delay slots
  char *action;               //!< Any other action to be performed (C/C++ block)
} ac_control_flow;

//! Instruction type used to build the decoder
typedef struct _ac_dec_instr{
  char *name;                 //!< Instruction class name
  int size;                   //!< Instruction size in bytes
  char *mnemonic;             //!< Instruction mnemonic
  char *asm_str;              //!< Assembly string to print the instruction
  char *format;               //!< Instruction format
  unsigned id;                //!< Instruction ID
  unsigned cycles;            //!< Used for Multi-cycle instructions
  unsigned min_latency;       //!< Used for cycle-count estimate
  unsigned max_latency;       //!< Used for cycle-count estimate
  ac_dec_list *dec_list;      //!< Sequence of decode passes
  ac_control_flow *cflow;     //!< Used for control flow instructions (jump/branch)
  struct _ac_dec_instr *next; //!< Next instruction
} ac_dec_instr;


//! Decode tree structure. It is used to decode one instruction
typedef struct _ac_decoder{
  ac_dec_list *check;           //!< Field to be checked.
  ac_dec_instr *found;          //!< Instruction detected if field checked (valid only when !NULL).
  struct _ac_decoder *subcheck; //!< Sub-decode phase (new field to be checked).
  struct _ac_decoder *next;     //!< Next field/value to be checked.
} ac_decoder;


typedef struct _ac_decoder_full{
  ac_decoder *decoder;
  ac_dec_format *formats;
  ac_dec_field *fields;
  ac_dec_instr *instructions;
  unsigned nFields;
} ac_decoder_full;


void MemoryError(char *fileName, long lineNumber, char *functionName);
void ShowError(char *msg);
void ShowDecField(ac_dec_field *f);
void ShowDecFormat(ac_dec_format *f);
void ShowDecodeList(ac_dec_list *l);
void ShowDecInstr(ac_dec_instr *i);
void ShowDecoder(ac_decoder *d, unsigned level);
ac_decoder_full *CreateDecoder(ac_dec_format *formats, ac_dec_instr *instructions);

ac_dec_format *FindFormat(ac_dec_format *formats, char *name);
ac_dec_instr *GetInstrByID(ac_dec_instr *instr, int id);
unsigned *Decode(ac_decoder_full *decoder, unsigned char *buffer, int quant);

#ifdef __cplusplus
}
#endif

#endif

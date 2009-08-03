/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_decoder_rt.H
 * @author    Marcus Bartholomeu
 *            Sandro Rigo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
 * 
 * @brief     ArchC Decoder header file.
 *            This file contains the structures needed for the ArchC
 *            Pre-processor to build  the decoder for the target ISA.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef _AC_DECODER_RT_HPP_
#define _AC_DECODER_RT_HPP_

#include <iostream>
#include <string>

using std::string;
using std::ostream;
using std::endl;

// ac_dec_format forward declaration
struct ac_dec_format;

// ac_decoder_full forward declaration
struct ac_decoder_full;

//! Field type used to build the decoder
class ac_dec_field {
public:
  string name;                //!< Field name
  int size;                   //!< Field size in bits
  int first_bit;              //!< First bit of the field inside the instruction
  int id;                     //!< Unique id that identifies this field
  long val;                   //!< Value of the field
  int sign;                   //!< Indicates whether the field is signed or not
  ac_dec_field* next;         //!< Next field

  /// Comparison operators
  int operator <  (ac_dec_field& rs) {
    return name <  rs.name;
  };
  int operator <= (ac_dec_field& rs) {
    return name <= rs.name;
  };
  int operator >  (ac_dec_field& rs) {
    return name >  rs.name;
  };
  int operator >= (ac_dec_field& rs) {
    return name >= rs.name;
  };
  int operator == (ac_dec_field& rs) {
    return name == rs.name;
  };

  /// Checks fields
  int CheckFields(const ac_dec_field& f2) const;

  /// Finds fields by name
  ac_dec_field* FindDecField(int id);

  /// Output operator
  friend ostream& operator << (ostream& os, ac_dec_field& adf);

  static ac_dec_field* PutIDs(ac_dec_format* formats, unsigned nFields);

};

//! Format type used to build the decoder
struct ac_dec_format {
  string name;                 //!< Format name
  int size;                    //!< Format size in bits
  ac_dec_field* fields;        //!< List of fields in this format
  ac_dec_format* next;         //!< Next format

  friend ostream& operator << (ostream& os, ac_dec_format& adf);

  static ac_dec_format* FindFormat(ac_dec_format *formats, const char *name);
};

//! Type used to build the decoder
struct ac_dec_list {
  string name;                  //!< Field name to be checked
  int id;                       //!< Field ID to be checked
  int value;                    //!< Value to find
  ac_dec_list* next;            //!< Next decode element
  
  friend ostream& operator << (ostream& os, ac_dec_list& adl);
};

//! Type used for control flow instructions
typedef struct control_flow {
  string cond;                 //!< Condition for control instruction
  string target;               //!< Target for the jump/branch
  int    delay_slot;           //!< Number of delay slots
  string delay_slot_cond;      //!< Condition to execute instructions in the delay slots
  string action;               //!< Any other action to be performed (C/C++ block)
} ac_control_flow;

//! Instruction type used to build the decoder
class ac_dec_instr {
public:
  string name;                 //!< Instruction class name
  int size;                   //!< Instruction size in bytes
  string mnemonic;             //!< Instruction mnemonic
  string asm_str;              //!< Assembly string to print the instruction
  string format;               //!< Instruction format
  unsigned id;                //!< Instruction ID
  unsigned cycles;            //!< Used for Multi-cycle instructions
  unsigned min_latency;       //!< Used for cycle-count estimate
  unsigned max_latency;       //!< Used for cycle-count estimate
  ac_dec_list* dec_list;      //!< Sequence of decode passes
  control_flow* cflow;     //!< Used for control flow instructions (jump/branch)
  ac_dec_instr* next; //!< Next instruction

  friend ostream& operator << (ostream& os, ac_dec_instr& adi);

  static ac_dec_instr* GetInstrByID(ac_dec_instr *instr, unsigned id);

};


//! Decode tree structure. It is used to decode one instruction
struct ac_decoder {
  ac_dec_list* check;           //!< Field to be checked.
  ac_dec_instr* found;          //!< Instruction detected if field checked (valid only when !NULL).
  ac_decoder* subcheck; //!< Sub-decode phase (new field to be checked).
  ac_decoder* next;     //!< Next field/value to be checked.

  void ShowDecoder(unsigned level);

  static ac_decoder* AddToDecoder(ac_decoder* decoder, ac_dec_instr* instruction, ac_dec_field* fields,
                                  ac_dec_format *fmt);

};

class ac_dec_prog_source {
public:
  //GetBits function
  virtual unsigned long long GetBits(unsigned char* buffer,
                                     int* quant, int last,
                                     int quantity, int sign) = 0;
};

struct ac_decoder_full {
  ac_decoder* decoder;
  ac_dec_format* formats;
  ac_dec_field* fields;
  ac_dec_instr* instructions;
  ac_dec_prog_source* prog_source;
  unsigned nFields;

  static ac_decoder_full* CreateDecoder(ac_dec_format* formats,
                                        ac_dec_instr* instructions,
                                        ac_dec_prog_source* source);

  unsigned* Decode(unsigned char *buffer, int quant);

};

void MemoryError(char *fileName, long lineNumber, char *functionName);
void ShowError(char *msg);

#endif // _AC_DECODER_RT_HPP_

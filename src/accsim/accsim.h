/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC Pre-processor generates tools for the described arquitecture
    Copyright (C) 2004  The ArchC Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

//////////////////////////////////////////////////////////////////
/*!\file accs.h                                                
  \brief ArchC Pre-processor for Compiled Simulation header file
  This file contains the structures and macro definitions    
  needed by the ArchC Pre-processor for Compiled Simulation   */
//////////////////////////////////////////////////////////////////

#ifndef _ACCS_H_
#define _ACCS_H_

extern int compsim;     //!< flag compiled simulation?



//!Structure to keep decoded instruction information.
//!When in preprocessor time, it is dinamicaly allocated, when in simulation
//!  time, it is staticaly allocated and there is also a pc_label element.
typedef struct {
  unsigned *dec_vector;
  int is_leader;
} instr_decode_t;

instr_decode_t **decode_table;      //!< Decoded program table




//Function Prototypes
int  accs_main();

int  accs_ReadHexProgram(char* prog_filename);
//void accs_ReadElfSection(char* elffile, char* scn_name);
int  accs_ReadElf(char* elffile);
int  accs_ReadObjdumpHex(char* prog_filename);

void accs_CreateMain();

void accs_CreateCompsimHeader();
void accs_CreateCompsimImpl();
void accs_CreateCompsimHeaderRegions();

void accs_CreateStorageHeader();
void accs_CreateStorageImpl();

void accs_CreateISAHeader();
void accs_CreateISAImpl();
void accs_CreateISAInitImpl();

void accs_CreateResourcesHeader();
void accs_CreateResourcesImpl();

void accs_CreateDelayHeader();
void accs_CreateDelayImpl();

void accs_CreateConfigHeader();

void accs_CreateISABehaviorHeader();

void accs_CreateSyscallHeader();
void accs_CreateSyscallImpl();

void accs_CreateArchSyscallHeader();

void accs_CreateMakefile();

void accs_CreateProgramMemoryHeader();
void accs_CreateProgramMemoryImpl();

void accs_CreateARCHHeader();
void accs_CreateARCHImpl();


// fast_* functions create files using the old but faster storage class

void fast_CreateMain();

void fast_CreateCompsimHeader();
void fast_CreateCompsimImpl();

void fast_CreateStorageHeader();
void fast_CreateStorageImpl();

void fast_CreateResourcesHeader();
void fast_CreateResourcesImpl();

void fast_CreateDelayHeader();
void fast_CreateDelayImpl();

void fast_CreateConfigHeader();

void fast_CreateSyscallHeader();
void fast_CreateSyscallImpl();

void fast_CreateMakefile();

void fast_CreateFileResourcesHeader();
void fast_CreateFileResourcesImpl();





void accs_CreateEmptyFiles(int quant, ...);
char *accs_Fields2Str(instr_decode_t* decoded_instr);
char *accs_SubstFields(char* expression, int j);
char *accs_SubstValue(char* str, char* var, unsigned val, int sign);


//Emit functions
void accs_EmitDecStruct( FILE* output);
int accs_EmitSyscalls(FILE* output, int j);
void accs_EmitInstr(FILE* output, int j);
void accs_EmitInstrBehavior(FILE* output, int j, ac_dec_instr *pinstr, int indent);
void accs_EmitInstrExtraTop(FILE* output, int j, ac_dec_instr *pinstr, int indent);
void accs_EmitInstrExtraBottom(FILE* output, int j, ac_dec_instr *pinstr, int indent);
void accs_EmitMakefileExtra(FILE* output);
void accs_EmitParmsExtra(FILE* output);


#endif /*_ACCS_H_*/

 

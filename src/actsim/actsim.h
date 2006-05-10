/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC simulator generator generates simulators for the described architecture
    Copyright (C) 2002-2004  The ArchC Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

/********************************************************/
/* actsim.h: The ArchC timed simulator generator.       */
/* Author: Sandro Rigo, Marilia Felippe Chiozo          */
/* Date: 16-07-2002, 30-01-2006                         */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
////////////////////////////////////////////////////////////////

/*!\file actsim.h
  \brief ArchC Timed Simulator Generator header file
  This file contains the structures and macro definitions
  needed by the ArchC Timed Simulator.                        */
////////////////////////////////////////////////////////////////

/** @defgroup actsim ArchC timed simulator generator
 * @{
 */

#ifndef _ACTSIM_H_
#define _ACTSIM_H_

#define _NEMDEFS_SINGLEINDENT_

#include <stdlib.h>
#include <stdio.h>
#include "ac_decoder.h"

/*Defining indentation */
#ifdef _NEMDEFS_SINGLEINDENT_
#define INDENT0  ""
#define INDENT1  " "
#define INDENT2  "  "
#define INDENT3  "   "
#define INDENT4  "    "
#define INDENT5  "     "
static const char* INDENT[] = {"", " ", "  ", "   ", "    ", "     ", "      ",
                               "       "};
#else
#define INDENT0  ""
#define INDENT1  "  "
#define INDENT2  "    "
#define INDENT3  "      "
#define INDENT4  "        "
#define INDENT5  "          "
static const char* INDENT[] = {"", "  ", "    ", "      ", "        ",
                               "          ", "            ", "              "};
#endif

#define CONF_MAX_LINE 256   //!< Maximum number of characters per line in archc.conf


#define WRITE_THROUGH   0x01    //!< Cache will use the write-through policy.
#define WRITE_BACK      0x02    //!< Cache will use the write-back policy.
#define WRITE_AROUND    0x20    //!< Cache will use the write-around policy.
#define WRITE_ALLOCATE  0x10    //!< Cache will use the write-allocate policy.

/** @defgroup confvar ArchC Configuration Variables
 * @ingroup actsim
 * These variables will be used by actsim to generate a makefile for models.
 * They are extracted from the config/archc.conf file.
 * @{
 */
char* ARCHC_PATH;     //!< Path where ArchC is installed.
char* SYSTEMC_PATH;   //!< Path where SystemC is installed.
char* TARGET_ARCH;    //!< Architecture of the host machine.
char* CC_PATH;        //!< C/C++ compiler path
char* OPT_FLAGS;      //!< Optimization flags to be passed to the compiler
char* DEBUG_FLAGS;    //!< Debugging flags to be passed to the compiler 
char* OTHER_FLAGS;    //!< Miscellaneous flags to be passed to the compiler
//@}

/*! Emit a doxygen-formated C comment */
#define COMMENT( I, args... )   fprintf(output, "%s//! ", I); fprintf(output, args); fprintf(output, "\n");

/*! Emit a Makefile-formated comment */
#define COMMENT_MAKE( args... ) fprintf(output, "# "); fprintf(output, args); fprintf(output, "\n");

/*! Issuing an Internal Error message */
#define AC_INTERNAL_ERROR( str ) fprintf(stderr, "ArchC ERROR: %s. File: %s Line: %d\n", str, __FILE__, __LINE__);

/*! Issuing an Error message */
#define AC_ERROR( str, ... ) fprintf(stderr, "ArchC ERROR: " str, ##__VA_ARGS__);
//#define AC_ERROR( str ) fprintf(stderr, "ArchC ERROR: %s ", str );

/*! Issuing an  message */
#define AC_MSG( str, ... ) fprintf(stdout, "ArchC: " str, ##__VA_ARGS__);
//#define AC_MSG( str ) printf("ArchC: ");printf str;

//! Enumeration type for command line options
enum _ac_cmd_options
{
 OPABI,
 OPDasm,
 OPDebug,
 /*OPDelay,*/
 OPDDecoder,
 OPHelp,
 /*OPQuiet,*/
 OPDecCache,
 OPStats,
 OPVerbose,
 /*OPVerify,*/
 /*OPVerifyTimed,*/
 OPVersion,
 /*OPEncoder,*/
 OPGDBIntegration,
 ACNumberOfOptions
};

typedef enum _ac_cmd_options_ ac_cmd_options;

//! Enumeration type for storage device types
enum _ac_sto_types {MEM, CACHE, ICACHE, DCACHE, REG, REGBANK, TLM_PORT,
                    TLM_INTR_PORT};

typedef enum _ac_sto_types ac_sto_types;

//! List of a Cache's parameters
typedef struct _ac_cache_parms
{
 char* str;                    //!< Used for string parameters.
 unsigned value;               //!< Used for numerical parameters.
 struct _ac_cache_parms* next; //!< Next element
} ac_cache_parms;

//! List of Storage Devices
typedef struct _ac_sto_list
{
 char* name;                   //!< Device name.
 char* format;                 //!< Device format. Only possible for registers.
 unsigned size;                //!< Size expressed in bytes. Used for memories and generic caches.
 unsigned width;               //!< Width of registers expressed in bits. Used for register banks.
 unsigned level;               //!< Memory hierachy level. 
 ac_sto_types type;            //!< Type of the device
 struct _ac_sto_list* higher;  //!< Points to the successor in the memory hierarchy.
 ac_cache_parms* parms;        //!< Parameter list used for (not generic) ac_cache  declarations.
 struct _ac_sto_list* next;    //!< Next element
} ac_sto_list;

//!List of Stages
typedef struct _ac_stg_list
{
 char* name;                   //!< Device name.
 unsigned id;                  //!< Stage Identification Number.
 struct _ac_stg_list* next;    //!< Next element
} ac_stg_list;

//!List of Pipelines
typedef struct _ac_pipe_list
{
 char* name;                   //!< Device name.
 unsigned id;                  //!< Pipe identification Number.
 ac_stg_list* stages;          //!< List of Stages
 struct _ac_pipe_list* next;   //!< Next element
} ac_pipe_list;

//! Used to check the endianess of the host machine.
typedef union
{
 int i;
 //char c[4];
 char c[sizeof(int)];
} endian;

//@}

///////////////////////////////////////
// Function Prototypes               //
///////////////////////////////////////

void print_comment(FILE* output, const char* description);

/** @defgroup createfunc Create Functions
 * @ingroup actsim
 * These functions create several files, composing
 * the timed simulator.
 * A function name tells which file it will create.
 * Header functions create .H files and Impl functions create
 * .cpp files.
 * @{
 */

void CreateArchHeader(void);                      //!< Creates the header file for <project>_arch class.
void CreateArchRefHeader(void);                   //!< Creates the header file for <project>_arch_ref class.
void CreateParmHeader(void);                      //!< Creates the header file for ArchC common parameters.
void CreateISAHeader(void);                       //!< Creates the header file for the ISA class.
void CreateProcessorHeader(void);                 //!< Creates the header file for the processor class.
void CreateStagesRefHeader(void);                 //!< Creates the header file for the stages references class.
void CreateStgHeader(ac_stg_list* stage_list, char* pipe_name);  //!< Creates the header files for pipeline stages.
void CreateRegsHeader(void);                      //!< Creates the header file for ArchC formatted registers.
void CreateCoverifHeader(void);                   //!< Creates the header file for ArchC co-verification class.
void CreateStatsHeaderTmpl(void);                 //!< Creates the header file for ArchC statistics collection class.
void CreateIntrHeader(void);                      //!< Creates the header file for interrupt handlers.
void CreateIntrMacrosHeader(void);                //!< Creates the header file for interrupt handler macros.
void CreateArchSyscallHeader(void);               //!< Creates the header file for ArchC model syscalls. 
void CreateMakefile(void);                        //!< Creates a Makefile for the ArchC model.

void CreateArchRefImpl(void);                    //!< Creates the .cpp file for <project>_arch_ref class.
void CreateStgImpl(ac_stg_list* stage_list, char* pipe_name); //!< Creates the .cpp file for pipeline stages.
void CreateProcessorImpl(void);                   //!< Creates the .cpp file for processor module class.
void CreateMainTmpl(void);                        //!< Creates the .cpp template file for the main function.
void CreateImplTmpl(void);                        //!< Creates the .cpp template file for behavior description.
void CreateArchGDBImplTmpl(void);                 //!< Creates the .cpp template file for GDB integration.
void CreateISAInitImpl(void);                     //!< Creates the .cpp ISA initialization file.
void CreateArchABIImplTmpl(void);                 //!< Creates the .cpp template file for ABI implementation.
void CreateStatsImplTmpl(void);                   //!< Creates the implementation file for ArchC statistics collection class.
void CreateRegsImpl(void);                        //!< Creates the .cpp template file for formatted registers.
void CreateIntrTmpl(void);                        //!< Creates the .cpp template file for interrupt handlers.
//@}

/** @defgroup emitfunc Emit Functions
 * @ingroup actsim
 * These functions are used by Create functions to emit code.
 * @{
*/
void EmitDecStruct(FILE* output);                 //!< Emits decoder structure initialization.
void EmitDecodification(FILE* output, int base_indent);       //!< Emits for instruction decodification
void EmitInstrExec(FILE* output, int base_indent);            //!< Emits code for executing an instruction behavior
void EmitFetchInit(FILE* output, int base_indent);            //!< Emits code used for initializing fetchs
void EmitMultiCycleProcessorBhv(FILE* output);    //!< Emits processor behavior for a multicycle processor.
void EmitPipeABIDefine(FILE* output);             //!< Emits the define that implements the ABI control for pipelined architectures
void EmitABIDefine(FILE* output, int base_indent);            //!< Emits the define that implements the ABI control for non-pipelined architectures
void EmitABIAddrList(FILE* output, int base_indent);          //!< Emits the calls for macros containing the list o address used for system calls
void EmitCacheDeclaration(FILE* output, ac_sto_list* pstorage, int base_indent);       //!< Emits code for ac_cache object declaration
void EmitCacheInitialization(char* output, ac_sto_list* pstorage);                     //!< Emits code for ac_cache object initialization
//@}

/** @defgroup utilitfunc Utility Functions
 * @ingroup actsim
 * These functions perform several tasks needed by Create and Emit functions to emit the simulator source code.
 * @{
 */
void ReadConfFile(void);                          //!< Reads archc.conf contents.
//@}

#endif /* _ACTSIM_H_ */

////////////////////////////////////////////////////////////////
//   This is used by DoxyGen !!!
///////////////////////////////////////////////////////////////
/*!\mainpage The ArchC Timed Simulator Generator Developers Guide

\section Introduction Introduction

<P>ArchC   is  an   architectural  description   language  based   on  <a
href="http://www.systemc.org">SystemC</a>. The  main goal in designing
ArchC is  to provide architecture  designers with a language  that can
speed up the  task of specifying and verifying  a new architecture, and
allow  automatic  synthesis of  processor  simulators, assemblers  and
compilers starting from ArchC  descriptions.  ArchC is being developed
at <a  href="http://www.lsc.ic.unicamp.br">Computer Systems Laboratory
(LSC)</a>,       which       is       part       of       the       <a
href="http://www.ic.unicamp.br">Institute  of   Computing</a>,  at  <a
href="http://www.unicamp.br">University of Campinas (UNICAMP)</a>.</p>

<p>This guide was prepared as a quick reference for the ArchC timed
simulator generator (actsim) implementation. Its  main goal is to provide a
starting point for new ArchC developers and maintainers. It is not appropriated
for ArchC users, who should refer to ArchC Language Reference Manual.</p>

\section Contact  Contact

If you have any comments or doubts, please refer to the
ArchC homepage: http://www.archc.org

*/
  ////////////////////////////////////////////////////////////////

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC simulator generator generates simulators for the described arquitecture
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
/* acsim.h: The ArchC simulator generator.              */
/* Author: Sandro Rigo                                  */
/* Date: 16-07-2002                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
////////////////////////////////////////////////////////////////

/*!\file acsim.h                                                
  \brief ArchC Simulator Generator header file                      
  This file contains the structures and macro definitions    
  needed by the ArchC Simulator.                              */
////////////////////////////////////////////////////////////////

/** @defgroup acsim ArchC simulator generator
 * @{
 */

#ifndef _ACSIM_H_
#define _ACSIM_H_

#include <stdlib.h>
#include <stdio.h>
#include "ac_decoder.h"

/*Defining indentation */
#define INDENT0  ""
#define INDENT1  "  "
#define INDENT2  "    "
#define INDENT3  "      "
#define INDENT4  "        "
#define INDENT5  "          "
static const char *INDENT[]={"","  ","    ","      ","        ","          ","            ","              "};

#define CONF_MAX_LINE 256   //!<Maximal number of characters per line in archc.conf


#define WRITE_THROUGH   0x01    //!<Cache will use the write-through policy.
#define WRITE_BACK      0x02    //!<Cache will use the write-back policy.
#define WRITE_AROUND    0x20    //!<Cache will use the write-around policy.
#define WRITE_ALLOCATE  0x10    //!<Cache will use the write-allocate policy.

/** @defgroup confvar ArchC Configuration Variables
 * @ingroup acsim
 * These variables will be used by acsim to generate a makefile for models.
 * They are extracted from the config/archc.conf file.
 * @{
 */
//char *ARCHC_PATH;     //!<Path where ArchC is installed.
char *SYSTEMC_PATH;   //!<Path where SystemC is installed.
char *TLM_PATH;       //!<Path to the TLM directory
char *TARGET_ARCH;    //!<Architecture of the host machine.
char *CC_PATH;        //!<C/C++ compiler path
char *OPT_FLAGS;      //!<Optimization flags to be passed to the compiler
char *DEBUG_FLAGS;    //!<Debugging flags to be passed to the compiler 
char *OTHER_FLAGS;    //!<Miscellaneous flags to be passed to the compiler
//@}

/*! Emit a doxygen-formated C comment */
#define COMMENT( I, args... )   fprintf( output, "%s//!", I); fprintf( output, args);fprintf( output, "\n");

/*! Emit a Makefile-formated comment */
#define COMMENT_MAKE( args... ) fprintf( output, "# "); fprintf( output, args);fprintf( output, "\n");

/*! Issuing an Internal Error message */
#define AC_INTERNAL_ERROR( str ) fprintf(stderr, "ArchC ERROR: %s. File: %s Line: %d\n", str, __FILE__, __LINE__);

/*! Issuing an Error message */
#define AC_ERROR( str, ...) fprintf(stderr, "ArchC ERROR: " str, ##__VA_ARGS__);
//#define AC_ERROR( str ) fprintf(stderr, "ArchC ERROR: %s ", str );

/*! Issuing an  message */
#define AC_MSG( str, ...) fprintf(stdout, "ArchC: " str, ##__VA_ARGS__);
//#define AC_MSG( str ) printf("ArchC: ");printf str;

//! Enumeration type for command line options
enum _ac_cmd_options {
  OPABI,
  OPDebug,
  OPDelay,
  OPDDecoder,
  OPHelp,
  OPDecCache,
  OPStats,
  OPVerbose,
  OPVersion,
  OPGDBIntegration,
  ACNumberOfOptions
};

typedef enum  _ac_cmd_options_ ac_cmd_options;

//! Enumeration type for storage device types
enum _ac_sto_types { MEM, CACHE, ICACHE, DCACHE, REG, REGBANK, TLM_PORT, TLM_INTR_PORT };

typedef enum _ac_sto_types ac_sto_types;

//! List of a Cache's parameters
typedef struct _ac_cache_parms{
  char *str;                    //!< Used for string parameters.
  unsigned value;                       //!< Used for numerical parameters.
  struct _ac_cache_parms *next; //!< Next element
} ac_cache_parms;

//! List of Storage Devices
typedef struct _ac_sto_list{
  char *name;                   //!< Device name.
  char *format;                 //!< Device format. Only possible for registers.
  unsigned size;                //!< Size expressed in bytes. Used for memories and generic caches.
  unsigned width;               //!< Width of registers expressed in bits. Used for register banks.
  unsigned level;               //!< Memory hierachy level. 
  ac_sto_types type;            //!< Type of the device
  struct _ac_sto_list* higher;  //!< Points to the successor in the memory hierarchy.
  ac_cache_parms *parms;        //!< Parameter list used for (not generic) ac_cache  declarations.
  struct _ac_sto_list *next;    //!< Next element
} ac_sto_list;

//!List of Stages
typedef struct _ac_stg_list{
  char *name;                   //!< Device name.
  unsigned id;                  //!< Stage Identification Number.
  struct _ac_stg_list *next;    //!< Next element
} ac_stg_list;

//!List of Pipelines
typedef struct _ac_pipe_list{
  char *name;                   //!< Device name.
  unsigned id;                  //!< Pipe identification Number.
  ac_stg_list *stages;          //!< List of Stages
  struct _ac_pipe_list *next;   //!< Next element
} ac_pipe_list;

//!Used to check the endianess of the host machine.
typedef union  {
  int i;
  char c[4];
}endian;

//@}

///////////////////////////////////////
// Function Prototypes               //
///////////////////////////////////////

void print_comment( FILE* output, char* description);

/** @defgroup createfunc Create Functions
 * @ingroup acsim
 * These functions create several files, composing
 * the behavioral simulator.
 * A function name tells which file it will create.
 * Header functions create .H files and Impl functions create
 * .cpp files.
 * @{
 */

void CreateArchHeader(void);                  //!< Creates the header file for ac_resources class.
void CreateArchImpl(void);                        //!< Creates the .cpp file for _arch class.
void CreateArchRefHeader(void);                  //!< Creates the header file for proj_arch_ref class.
void CreateArchRefImpl(void);                  //!< Creates the .cpp file for proj_arch_ref class.
void CreateISAHeader(void);                       //!< Creates the header file for the AC_ISA derived class.
void CreateStgHeader(ac_stg_list* stage_list, char* pipe_name);  //!< Creates the header files for pipeline stages.
void CreateProcessorHeader(void);                 //!< Creates the header file for the processor module.
void CreateParmHeader(void);                      //!< Creates the header file for ArchC common parameters.
void CreateRegsHeader(void);                      //!< Creates the header file for ArchC formatted registers.
void CreateStatsHeaderTmpl(void);                     //!< Creates the header file for ArchC statistics collection class.    
//!Create the implementation file for ArchC statistics collection class.
void CreateStatsImplTmpl();
void CreateArchSyscallHeader(void);               //!< Creates the header file for ArchC model syscalls. 
void CreateIntrHeader(void);                      //!< Creates the header file for interrupt handlers.
void CreateIntrMacrosHeader(void);                //!< Creates the header file for interrupt handler macros.
void CreateMakefile(void);                        //!< Creates a Makefile for teh ArchC nodel.
void CreateStgImpl(ac_stg_list* stage_list, char* pipe_name); //!< Creates the .cpp file for pipeline stages.
void CreateRegsImpl(void);                        //!< Creates the .cpp template file for formatted registers.
void CreateImplTmpl(void);                        //!< Creates the .cpp template file for behavior description.
void CreateIntrTmpl(void);                        //!< Creates the .cpp template file for interrupt handlers.
void CreateMainTmpl(void);                        //!< Creates the .cpp template file for the main function.
void CreateProcessorImpl(void);                   //!< Creates the .cpp file for processor module.

/** @defgroup emitfunc Emit Functions
 * @ingroup acsim
 * These functions are used by Create functions to emit code.
 * @{
*/
void EmitGenInstrClass(FILE *output);             //!< Emit class declaration for generic instruction.
void EmitFormatClasses(FILE *output);             //!< Emit class declarations for formats.
void EmitInstrClasses(FILE *output);              //!< Emit class declarations for instructions.
void EmitDecStruct( FILE* output);                //!< Emit decoder structure initialization.
void EmitPipeUpdateMethod( FILE *output);         //!< Emit reg update method for pipelined architectures.
void EmitMultiPipeUpdateMethod( FILE *output);    //!< Emit reg update method for multi-pipelined architectures.
void EmitUpdateMethod( FILE *output);             //!< Emit reg update method for non-pipelined architectures.
void EmitMultiCycleProcessorBhv(FILE *output);    //!< Emit processor behavior for a multicycle processor.
void EmitProcessorBhv( FILE *output);             //!< Emit processor behavior for a single-cycle processor.
void EmitProcessorBhv_ABI( FILE *output);         //!< Emit processor behavior for a single-cycle processor with ABI provided.
void EmitABIAddrList( FILE *output, int base_indent);           //!< Emit the calls for macros containing the list o address used for system calls
void EmitABIDefine( FILE *output);                //!< Emit the define that implements the ABI control for non-pipelined architectures
void EmitPipeABIDefine( FILE *output);            //!< Emit the define that implements the ABI control for pipelined architectures
void EmitInstrExec(FILE *output, int base_indent);              //!< Emit code for executing an instruction behavior
void EmitDecodification(FILE *output, int base_indent);         //!< Emit for instruction decodification
void EmitFetchInit(FILE *output, int base_indent);              //!< Emit code used for initializing fetchs
void EmitCacheDeclaration(FILE *output, ac_sto_list* pstorage, int base_indent);       //!< Emit code for ac_cache object declaration
//@}

/** @defgroup utilitfunc Utility Functions
 * @ingroup acsim
 * These functions perform several tasks needed by Create and Emit functions to emit the simulator source code.
 * @{
 */
void ReadConfFile(void);                          //!< Read archc.conf contents.
//@}


#endif /*_ACSIM_H_*/

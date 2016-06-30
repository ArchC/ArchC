/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      acsim.h
 * @author    Sandro Rigo
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
 * @brief     ArchC Simulator Generator header file
 *            This file contains the structures and macro definitions
 *            needed by the ArchC Simulator.
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

/** @defgroup acsim ArchC simulator generator
 * @{
 */

#ifndef _ACSIM_H_
#define _ACSIM_H_

#include <stdlib.h>
#include <stdio.h>
#include "ac_decoder.h"
#include "ac_tools_common.h"

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
  OPNOABI,
  OPDebug,
#if HLT_SUPPORT
  OPHLTrace,
#endif
  OPDelay,
  OPDDecoder,
  OPHelp,
  OPDecCache,
  OPStats,
  OPVerbose,
  OPVersion,
  OPGDBIntegration,
  OPWait,
  OPDTC,
  OPSysJump,
  OPForcedInline,
  OPLongJmpStop,
  OPIndexFix,
  OPPCAddress,
  OPFullDecode,
  OPCurInstrID,
  OPPower,
  ACNumberOfOptions,
};

typedef enum  _ac_cmd_options_ ac_cmd_options;


enum CacheType {
        WriteBack,
        WriteThrough
};

static const char *CacheName[] = {
  [WriteBack] = "ac_write_back_cache",
  [WriteThrough] = "ac_write_through_cache"
};

enum CacheReplacementPolicy {
  FIFO,
  Random,
  PLRUM,
  LRU,
  None
};

static const char *ReplacementPolicyName[] = {
  [FIFO] = "ac_fifo_replacement_policy",
  [Random] = "ac_random_replacement_policy",
  [PLRUM] = "ac_plrum_replacement_policy",
  [LRU] = "ac_lru_replacement_policy",
  [None] = "ac_fifo_replacement_policy" // placeholder
};


struct CacheObject {
  enum CacheType type;
  unsigned block_count; // index size * associativity
  unsigned block_size;
  unsigned associativity;
  enum CacheReplacementPolicy replacement_policy;
};




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
void CreateArchHeader(void);                      //!< Creates the header file for ac_resources class.
void CreateArchImpl(void);                        //!< Creates the .cpp file for _arch class.
void CreateArchRefHeader(void);                   //!< Creates the header file for proj_arch_ref class.
void CreateArchRefImpl(void);                     //!< Creates the .cpp file for proj_arch_ref class.
void CreateISAHeader(void);                       //!< Creates the header file for the AC_ISA derived class.
void CreateProcessorHeader(void);                 //!< Creates the header file for the processor module.
void CreateParmHeader(void);                      //!< Creates the header file for ArchC common parameters.
void CreateRegsHeader(void);                      //!< Creates the header file for ArchC formatted registers.
void CreateStatsHeaderTmpl(void);                 //!< Creates the header file for ArchC statistics collection class.
void CreateStatsImplTmpl();                       //!< Create the implementation file for ArchC statistics collection class.
void CreateArchSyscallHeader(void);               //!< Creates the header file for ArchC model syscalls.
void CreateArchSyscallTmpl(void);               //!< Creates the header file for ArchC model syscalls.
void CreateIntrHeader(void);                      //!< Creates the header file for interrupt handlers.
void CreateIntrMacrosHeader(void);                //!< Creates the header file for interrupt handler macros.
void CreateMakefile(void);                        //!< Creates a Makefile for the ArchC model.
void CreateRegsImpl(void);                        //!< Creates the .cpp template file for formatted registers.
void CreateImplTmpl(void);                        //!< Creates the .cpp template file for behavior description.
void CreateIntrTmpl(void);                        //!< Creates the .cpp template file for interrupt handlers.
void CreateMainTmpl(void);                        //!< Creates the .cpp template file for the main function.
void CreateProcessorImpl(void);                   //!< Creates the .cpp file for processor module.


void CreateIntrTLM2Header(void); /******/
void CreateIntrTLM2MacrosHeader(void); /*****/
void CreateIntrTLM2Tmpl(void); /*****/

/** @defgroup emitfunc Emit Functions
 * @ingroup acsim
 * These functions are used by Create functions to emit code.
 * @{
*/
void EmitUpdateMethod( FILE *output, int base_indent );                            //!< Emit reg update method for non-pipelined architectures.
void EmitProcessorBhv( FILE *output, int base_indent);                             //!< Emit processor behavior for a single-cycle processor.
void EmitInstrExec(FILE *output, int base_indent);                                 //!< Emit code for executing an instruction behavior
void EmitDecodification(FILE *output, int base_indent);                            //!< Emit for instruction decodification
void EmitFetchInit(FILE *output, int base_indent);                                 //!< Emit code used for initializing fetchs
void EmitCacheDeclaration(FILE *output, ac_sto_list* pstorage, int base_indent);   //!< Emit code for ac_cache object declaration
void EmitDecCache(FILE *output, int base_indent);                                  //!< Emits a Decoder Cache Structure
void EmitDecCacheAt(FILE *output, int base_indent);                                //!< Emits a Decoder Cache Attribution
void EmitDispatch(FILE *output, int base_indent);                                  //!< Emits the Dispatch Function used by Threading
void EmitVetLabelAt(FILE *output, int base_indent);                                //!< Emits the Vector with Address of the Interpretation Routines used by Threading
//@}

/** @defgroup utilitfunc Utility Functions
 * @ingroup acsim
 * These functions perform several tasks needed by Create and Emit functions to emit the simulator source code.
 * @{
 */
void ReadConfFile(void);                          //!< Read archc.conf contents.
void ParseCache(ac_sto_list *cache_in);
void CacheClassDeclaration(ac_sto_list *storage);
void MemoryClassDeclaration(ac_sto_list *memory);
void TLMMemoryClassDeclaration(ac_sto_list *memory);
void EnumerateCaches(void);
void GetFetchDevice(void);
void GetLoadDevice(void);
void GetFirstLevelDataDevice(void);


//@}


#endif /*_ACSIM_H_*/

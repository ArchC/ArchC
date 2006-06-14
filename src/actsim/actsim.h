/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      actsim.h
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
 * @date      Tue, 06 Jun 2006 18:05:49 -0300
 *
 * @brief     The ArchC timed simulator generator
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

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
#include "ac_tools_common.h"

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

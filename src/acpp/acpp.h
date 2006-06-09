/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      acpp.h
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Alexandro Baldassin
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
 * @brief     ArchC Pre-processor header file
 *
 *  This file contains wrapper functions to interface with
 *  the GNU bison/flex files. In the future it should scale
 *  to support an intermediate file representation.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/** @defgroup acpp_group The ArchC pre-processor (acpp)
 *
 * The ArchC pre-processor module contains the lexer and parser
 * definitions (GNU lex and bison) and data types filled by
 * the semantic action rules.
 * @{
 */


#ifndef _ACCP_H_
#define _ACCP_H_

#include "core_actions.h"
#include "asm_actions.h"

extern char *project_name;
extern char *isa_filename;
extern int  wordsize;
extern int  fetchsize;
extern int  fetchbuffersize;
extern int  ac_tgt_endian;

/* Function prototypes */

/*!
 * Initialises the preprocessor.
 * Always call this function before doing any other acpp function
 * call.
 *
 * \param[in] force_asm_syntax
 *            1- forces the new assembly syntax (need by the binary utilities generation tool)
 *            0- accepts an old and lazy assembly syntax (compatible with old models)
 */
extern void acppInit(int force_asm_syntax);

/*!
 * Loads the file to be used by AcppRun()
 *
 * \parm[in] filename name of the file to be parsed
 * \return 1 if file is open, 0 otherwise
 */
extern int acppLoad(char *filename);

/*!
 * Unloads any previously loaded file.
 *
 */
extern void acppUnload();

/*!
 * Parses the file loaded with acppLoad().
 * After returning from this function, the parser will have filled the main
 * data structures (and hence they can be used by generation tools)
 *
 * \return  1 if an error ocurred while parsing, 0 otherwise
 */
extern int acppRun();

/** @} */

#endif /*_ACCP_H_*/

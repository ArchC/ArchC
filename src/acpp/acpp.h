/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC Pre-processor generates tools for the described arquitecture
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
/* acpp.h: The ArchC pre-processor.                     */
/* Author: Sandro Rigo                                  */
/* Date: 16-07-2002                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

/*! \file acpp.h
 * \brief ArchC Pre-processor header file.
 *   
 *  This file contains wrapper functions to interface with 
 *  the GNU bison/flex files. In the future it should scale
 *  to support an intermediate file representation.
 */

/*! \defgroup acpp_group The ArchC pre-processor (acpp)
 * 
 * The ArchC pre-processor module contains the lexer and parser
 * definitions (GNU lex and bison) and the data types filled by
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
extern int  ac_tgt_endian;

/* Function prototypes */

/*! 
 * Initialises the pre-processor.
 * Always call this functions before doing any other acpp function
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
extern int  acppLoad(char *filename);

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
extern int  acppRun();


/** @} */

#endif /*_ACCP_H_*/

 

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
/* Acpp.c: The ArchC pre-processor.                     */
/* Author: Sandro Rigo                                  */
/* Date: 16-07-2002                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/
//////////////////////////////////////////////////////////
/*!\file acpp.c
  \brief The ArchC pre-processor.

  This file contains wrapper functions to interface with 
  the GNU bison/flex files. In the future it should scale
  to support an intermediate file representation.
*/
//////////////////////////////////////////////////////////

#include "acpp.h"
/* #include <stdlib.h> */
#include <stdio.h>

extern int yyparse();
extern FILE *yyin;
//extern int *yydebug 
extern int line_num;
extern int force_setasm_syntax;

/*!
   Initializes the pre-processor. Always call this before any other Acpp calls.
 */
void acppInit(int force_asm_syntax)
{
  //Initialize to 1 for parser debug
  //   *yydebug =1; 
  force_setasm_syntax = force_asm_syntax;

  project_name = NULL;
  isa_filename = NULL;
  wordsize = 0;
  fetchsize = 0;
  ac_tgt_endian = 1; /* defaults to big endian */


  yyin = NULL;
}


/*!
   Loads the file to be used by the AcppRun() call.
*/
int acppLoad(char *filename)
{
  acppUnload();
  return (int) (yyin = fopen(filename, "r" ));
}


/*!
   Unloads any previous loaded file.
*/
void acppUnload()
{
  if (yyin != NULL) fclose(yyin);
  yyin = NULL;
}


/*!
  Calls the parser using the file associated with AcppLoad().
*/
int acppRun()
{
  line_num = 1;
  return yyparse();
}



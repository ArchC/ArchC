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
/* Acpp.h: The ArchC pre-processor.                     */
/* Author: Sandro Rigo                                  */
/* Date: 16-07-2002                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
////////////////////////////////////////////////////////////////

/*!\file acpp.h                                                
  \brief ArchC Pre-processor header file                      
  This file contains the structures and macro definitions    
  needed by the ArchC Pre-processor.                        */
////////////////////////////////////////////////////////////////


#ifndef _ACCP_H_
#define _ACCP_H_

///////////////////////////////////////
// Function Prototypes               //
///////////////////////////////////////
extern void acppInit();
extern int acppLoad(char *filename);
extern void acppUnload();
extern int acppRun();


#endif /*_ACCP_H_*/

 

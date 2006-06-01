/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      gas.h
 * @author    Alexandro Baldassin (UNICAMP)
 *            Daniel Casarotto (UFSC)
 *            Max Schultz (UFSC)
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 *            System Design Automation Lab (LAPS)
 *            INE-UFSC
 *            http://www.laps.inf.ufsc.br/
 * 
 * @version   1.0
 * @date      Thu, 01 Jun 2006 14:28:06 -0300
 * 
 * @brief     GNU assembler related code
 * 
 * GAS module generates target-specific code to retarget the 
 * GNU assembler.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */


/** @defgroup gas_group GNU Assembler
 * @ingroup binutils_group
 *  
 * @{
 */

#ifndef _GAS_H_
#define _GAS_H_

extern int CreateEncodingFunc(const char *encfunc_filename);
extern int CreateGetFieldSizeFunc(const char *getfsz_filename);
extern int CreateGetInsnSizeFunc(const char *insnsz_filename);


#endif /* _GAS_H_ */

/* @} */

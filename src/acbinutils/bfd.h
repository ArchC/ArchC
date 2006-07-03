/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      bfd.h
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
 * @brief     BFD library related code
 * 
 * The BFD module deals with the aspects involved in retargeting
 * the BFD library.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/** @defgroup bfd_group BFD library
 * @ingroup binutils_group
 *
 * @{
 */

#ifndef _BFD_H_
#define _BFD_H_

extern void create_relocation_list();
extern int CreateRelocIds(const char *relocid_filename); 
extern int CreateRelocHowto(const char *reloc_howto_filename);
extern int CreateRelocMap(const char *relocmap_filename);
extern int CreateGetFieldValueFunc(const char *filename);


#endif /* _BFD_H_ */

/* @} */

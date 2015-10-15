/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      ac_tools_common.h
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
 * @date      Wed, 07 Jun 2006 16:34:27 -0300
 *
 * @brief     Common definitions shared among the ArchC tools.
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

#ifndef _AC_TOOLS_COMMON_H_
#define _AC_TOOLS_COMMON_H_

#include <stdbool.h>
#include "ac_decoder.h"

//! Enumeration type for storage device types
enum _ac_sto_types {MEM, CACHE, ICACHE, DCACHE, REG, REGBANK, TLM_PORT,
                    TLM_INTR_PORT, TLM2_PORT, TLM2_NB_PORT, TLM2_INTR_PORT};

typedef enum _ac_sto_types ac_sto_types;

//! List of a Cache's parameters
typedef struct _ac_cache_parms
{
 char* str;                    //!< Used for string parameters.
 unsigned value;               //!< Used for numerical parameters.
 struct _ac_cache_parms* next; //!< Next element.
} ac_cache_parms;

//! List of Storage Devices
typedef struct _ac_sto_list
{
 char* name;                   //!< Device name.
 char* format;                 //!< Device format. Only possible for registers.
 unsigned size;                //!< Size expressed in bytes. Used for memories and generic caches.
 unsigned width;               //!< Width of registers expressed in bits. Used for register banks.
 unsigned level;               //!< Memory hierachy level.
 ac_sto_types type;            //!< Type of the device.
 bool has_memport;
 struct _ac_sto_list* higher;  //!< Points to the successor in the memory hierarchy.
 ac_cache_parms* parms;        //!< Parameter list used for (not generic) ac_cache  declarations.
 struct _ac_sto_list* next;    //!< Next element.
 struct CacheObject *cache_object;
 char *class_declaration;      //!< String with class name and template parameters

} ac_sto_list;

//! List of Stages.
typedef struct _ac_stg_list
{
 char* name;                   //!< Device name.
 unsigned id;                  //!< Stage Identification Number.
 struct _ac_stg_list* next;    //!< Next element.
} ac_stg_list;

//! List of Pipelines.
typedef struct _ac_pipe_list
{
 char* name;                   //!< Device name.
 unsigned id;                  //!< Pipe identification Number.
 ac_stg_list* stages;          //!< List of Stages.
 struct _ac_pipe_list* next;   //!< Next element.
} ac_pipe_list;

//! List of instruction references.
typedef struct _ac_instr_ref_list
{
 ac_dec_instr* instr;
 struct _ac_instr_ref_list* next;
} ac_instr_ref_list;

//! List of groups.
typedef struct _ac_grp_list
{
 char* name; //!< Group name.
 unsigned id; //!< Group identification number.
 ac_instr_ref_list* instrs; //!< Instruction reference list.
 struct _ac_grp_list* next; //!< Pointer to next element.
} ac_grp_list;

//! Used to check the endianness of the host machine.
typedef union
{
 int i;
 char c[sizeof(int) / sizeof(char)];
} endian;

#endif

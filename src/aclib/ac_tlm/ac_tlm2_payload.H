/**
 * @file      ac_tlm2_payload.H
 * @author    Ivan Sichmann Freitas
 * @author    Liana Duenha
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @brief     Defines the ArchC TLM 2.0 payload abstraction.
 * 
 * @attention Copyright (C) 2002-2012 --- The ArchC Team
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

#ifndef _AC_TLM2_PAYLOAD_H_
#define _AC_TLM2_PAYLOAD_H_

#include <tlm.h>

using tlm::tlm_generic_payload;
using tlm::tlm_command;

/// Alias to the generic payload class
typedef tlm_generic_payload ac_tlm2_payload;


#endif // _AC_TLM2_PAYLOAD_H_

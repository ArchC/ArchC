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
 */

#ifndef _AC_TLM2_PAYLOAD_H_
#define _AC_TLM2_PAYLOAD_H_

#include <tlm.h>

using tlm::tlm_generic_payload;
using tlm::tlm_command;

/// Alias to the generic payload class
typedef tlm_generic_payload ac_tlm2_payload;


#endif // _AC_TLM2_PAYLOAD_H_

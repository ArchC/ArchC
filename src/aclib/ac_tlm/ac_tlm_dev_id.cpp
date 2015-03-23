/**
 * @file      ac_tlm_dev_id.cpp
 * @author    Thiago Massariolli Sigrist   
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 * 
 * @version   2.0beta2
 * @date      Fri, 05 May 2006 18:45:12 -0300
 * 
 * @brief     Defines a class that uniquely identifies a device.
 * 
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
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

//////////////////////////////////////////////////////////////////////////////

// Standard includes

// SystemC includes

// ArchC includes
#include "ac_tlm_dev_id.H"

//////////////////////////////////////////////////////////////////////////////

// using statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

// Constructors

/// ac_tlm_dev_id default constructor
ac_tlm_dev_id::ac_tlm_dev_id() : dev_id_(counter_++)
{}

//////////////////////////////////////////////////////////////////////////////

// Members
int ac_tlm_dev_id::counter_ = 0;

//////////////////////////////////////////////////////////////////////////////

// Methods

//////////////////////////////////////////////////////////////////////////////

// Destructors

//////////////////////////////////////////////////////////////////////////////


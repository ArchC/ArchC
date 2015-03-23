/**
 * @file      mips1_gdb_funcs.cpp
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     
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

#include "mips1.H"

int mips1::nRegs(void) {
   return 73;
}


ac_word mips1::reg_read( int reg ) {
  /* general purpose registers */
  if ( ( reg >= 0 ) && ( reg < 32 ) )
    return RB.read( reg );
  else {
    if ( ( reg >= 33 ) && ( reg < 35 ) )
      return RB.read( reg - 1 );
    else
      /* pc */
      if ( reg == 37 )
        return ac_resources::ac_pc;
  }

  return 0;
}


void mips1::reg_write( int reg, ac_word value ) {
  /* general purpose registers */
  if ( ( reg >= 0 ) && ( reg < 32 ) )
    RB.write( reg, value );
  else
    {
      /* lo, hi */
      if ( ( reg >= 33 ) && ( reg < 35 ) )
        RB.write( reg - 1, value );
      else
        /* pc */
        if ( reg == 37 )
          ac_resources::ac_pc = value;
    }
}


unsigned char mips1::mem_read( unsigned int address ) {
  return ac_resources::IM->read_byte( address );
}


void mips1::mem_write( unsigned int address, unsigned char byte ) {
  ac_resources::IM->write_byte( address, byte );
}

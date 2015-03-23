/**
 * @file      ac_gdb_interface.H
 * @author    Daniel Cabrini Hauagge    <ra008388@ic.unicamp.br>
 *            Gustavo Sverzut Barbieri  <ra008849@ic.unicamp.br>
 *            Joao Victor Andrade Neves <ra008951@ic.unicamp.br>
 *            Rafael Dantas de Castro   <ra009663@ic.unicamp.br>
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     GDB Support for ArchC simulators.
 *            This file provides the interface that must be implemented 
 *            by each processor. This interface handles the architecture
 *            specific details.
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
 * \note When modifing this file respect:
 * \li License
 * \li Previous author names. Add your own after current ones.
 * \li Coding style (basically emacs style)
 * \li Commenting style. This code use doxygen (http://www.doxygen.org)
 *     to be documented.
 *
 * \todo  This class should be replaced for standard STL element.
 */

/**
 * \defgroup GDB GDB Support for Simulators
 * Simulators built with ArchC can use the GDB protocol very easyly. Just 
 * implement a few processor dependent methods for the interface and the 
 * simulator will be able to respond to GDB, allowing users to debug 
 * software inside the simulator.
 *
 * The developer should implement methods to interface the architecture and GDB,
 * mapping registers and memory to the desired GDB format. These  methods,
 * explained in detail below, should be implemented in a file named after
 * your processor: \b PROC_gdb_funcs.cpp, given your processor
 * is implemented in \b PROC.ac. Then \e "make distclean" your 
 * simulator, run acsim with \e -gdb, and make it again. Acsim with \e -gdb 
 * will make your simulator class inherit from \ref AC_GDB_Interface and 
 * include your \b \<PROCESSOR_NAME\>_gdb_funcs.cpp, so using those 
 * functions.
 * 
 * When built, the simulator will handle the \e --gdb option, waiting for the GDB
 * to connect at port 5000 (port defined at compile time, changeable with 
 * \c -DPORT_NUM=\<YOUR_PORT\>, or defined at run time, using 
 * \e --gdb=\<YOUR_PORT\>).
 *
 * \section register Register Support
 * \par
 *		You must implement AC_GDB_Interface::nRegs(), 
 * AC_GDB_Interface::reg_read() and AC_GDB_Interface::reg_write(), so
 * the simulator can send the read  and  write register packets to GDB.   You
 * must check GDB documentation to learn the order the registers  should be provided, and map
 * them in \c reg_read() and \c reg_write(). Then define the number of 
 * registers GDB expects to receive/send by using \c nRegs(). The order is defined by the
 * \c REGISTER_RAW_SIZE and \c REGISTER_NAME macros. Please  read the
 * \e "info gdb", section \e "Remote Protocol",  nodes \e "Packets"  and  
 * \e "Register Packet Format"  before continue.
 *
 * \par Example:
 *	       If you have one bank with general purpose registers (\c RB_GP),
 * one with floating  point  registers (\c RB_FP) and  one  with status 
 * register (\c RB_S). If GDB expects the packets in the order: 32 general purpose 
 * registers, 32 floating point registers  and  8 status register and \c PC, 
 * your functions should be like: 
 *
 * \code
 *
 *      int YOUR_CLASS::nRegs( void ) {
 *	   return 73;
 *      }
 *
 *	ac_word YOUR_CLASS::reg_read( int reg ) {
 *         // General Purpose
 *         if ( ( reg >= 0 ) && ( reg < 32 ) )
 *            return RB_GP.read( reg );
 *
 *         // Floating Point
 *	   else if ( ( reg >= 32 ) && ( reg < 64 ) )
 *            return RB_FP.read( reg - 32 );
 *
 *         // Status
 *         else if ( ( reg >= 64 ) && ( reg < 72 ) )
 *            return RB_SR.read( reg - 64 );
 *
 *         // Program Counter
 *         else if ( reg == 72 )
 *            return ac_resources::ac_pc;
 *
 *         return 0; // unmaped register? return 0
 *	}
 *
 *      void YOUR_CLASS::reg_write( int reg, ac_word value ) {
 *         // General Purpose
 *         if ( ( reg >= 0 ) && ( reg < 32 ) )
 *            RB_GP.write( reg, value );
 *
 *         // Floating Point
 *         else if ( ( reg >= 32 ) && ( reg < 64 ) )
 *            RB_FP.write( reg - 32, value );
 *
 *         // Status
 *         else if ( ( reg >= 64 ) && ( reg < 72 ) )
 *            RB_SR.write( reg - 64, value );
 *
 *         // Program Counter
 *         else if ( reg == 72 )
 *            ac_resources::ac_pc = value;
 *      }
 *
 * \endcode
 *
 * \section mem Memory Support
 * \par
 *              You must implement  AC_GDB_Interface::mem_read()  and  
 * AC_GDB_Interface::mem_write() to read and write memory regions.
 *
 * \par Example:
 *		If you use just  one  memory  bank  (no  separated  data  and
 * instruction memory), it's easy as follow:
 *
 * \code
 *
 *      unsigned char YOUR_CLASS::mem_read( unsigned int address ) {
 *         return ac_resources::IM->read_byte( address );
 *      }
 *
 *      void YOUR_CLASS::mem_write( unsigned int address,
 *                                  unsigned char byte) {
 *         ac_resources::IM->write_byte( address, byte );
 *      }
 *
 * \endcode
 *
 * \sa \ref AC_GDB_Interface
 * \sa \ref mips1_gdb_funcs.cpp
 * \sa \ref sparcv8_gdb_funcs.cpp
 * \sa Gdb info page, section "Remote Protocol", node "Register Packet Format".
 * \sa Gdb source package, file gdb/remote-mips.c, mips_map_regno(). (http://sources.redhat.com/cgi-bin/cvsweb.cgi/~checkout~/src/gdb/remote-mips.c?&cvsroot=src)
 * \sa Gdb source package, file gdb/sparc-stub.c, regnames enumerate. (http://sources.redhat.com/cgi-bin/cvsweb.cgi/~checkout~/src/gdb/sparc-stub.c?&cvsroot=src)
 *
 */

/** \example mips1_gdb_funcs.cpp
 * MIPS1 implementation of GDB interface.
 */
/** \example sparcv8_gdb_funcs.cpp
 * SPARC v8 implementation of GDB interface.
 */


#ifndef _AC_GDB_INTERFACE_H_
#define _AC_GDB_INTERFACE_H_

#include "archc.H"

/* 
 * NOTE: 
 *
 * Declaring a method:
 *         virtual method=0
 * makes the class pure virtual, meaning that all methods MUST be implemented.
 */

/** AC_GDB_Interface
 * Interface for processor dependent methods to the GDB protocol.
 */
template <typename ac_word>
class AC_GDB_Interface {
public:

  /* Processor Feature Support */

  /**
   * Returns whether the processor is big-endian.
   *
   * @return false when simulated processor is little-endian, true when big-endian.
   */
  virtual bool get_ac_tgt_endian() = 0;

  /**
   * Stops the processor.
   */
  virtual void ac_stop() = 0;

  /* Register Support */

  /**
   * How many registers are there in the packet sent to gdb. Check GDB doc.
   *
   * \return how many registers the packet has.
   */  
  virtual int nRegs() = 0; 
  
  /**
   * Return register value according to GDB stub packet order.
   *
   * \param reg register number according to GDB stub packet order. Check GDB doc.
   *
   * \return register value.
   */
  virtual ac_word reg_read( int reg ) = 0;
  
   /**
    * Write register value according to GDB stub packet order.
    *
    * \param reg register number according to GDB stub packet order. Check GDB doc.
    * \param value what to be written.
    */
  virtual void reg_write( int reg, ac_word value ) = 0;
  

  /**
   * Writes address value to the processor ac_pc (Default Program Counter) register.
   *
   * @param value Address value to be written in the ac_pc register.
   */
  virtual void set_ac_pc(unsigned int value) = 0;

  /* Memory Access *************************************************************/
  
  /**
   * Read memory byte.
   *
   * \param address where to get byte.
   *
   * \return byte from address.
   */
  virtual unsigned char  mem_read( unsigned int address ) = 0;
  
  /**
   * Write memory byte.
   *
   * \param address where to write byte.
   * \param byte what to write.
   */
  virtual void mem_write( unsigned int address, unsigned char byte ) = 0;
};

#endif /* _AC_GDB_INTERFACE_H_ */

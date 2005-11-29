/** \file ac_gdb.H
 * GDB Support for ArchC simulators.
 *
 *    This is just the header file. Look for documentation in ac_gdb.cpp or in 
 * the doxygen auto-generated files.
 *
 ******************************************************************************
 *
 * \note When modifing this file respect:
 * \li License
 * \li Previous author names. Add your own after current ones.
 * \li Coding style (basically emacs style)
 * \li Commenting style. This code use doxygen (http://www.doxygen.org)
 *     to be documented.
 *
 ******************************************************************************
 *                                                                             
 * LICENSE:
 *    GNU GPL --- General Public License, version 2.0 or greater.
 *    See archc/COPYING for more information.
 *
 * \author Daniel Cabrini Hauagge    <ra008388@ic.unicamp.br>
 * \author Gustavo Sverzut Barbieri  <ra008849@ic.unicamp.br>
 * \author Joao Victor Andrade Neves <ra008951@ic.unicamp.br>
 * \author Rafael Dantas de Castro   <ra009663@ic.unicamp.br>
 *
 ******************************************************************************
 */
#ifndef _AC_GDB_H_
#define _AC_GDB_H_

#include "breakpoints.H"
#include "ac_gdb_interface.H"
#include "ac_resources.H"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

#ifndef BREAKPOINTS
#   define BREAKPOINTS 200
#endif

#ifndef GDB_BUFFERSIZE
#   define GDB_BUFFERSIZE 2048
#endif

#ifdef DEBUG
#   define debug( x ) \
   cerr << "debug:" << __FILE__ << ":" << __LINE__ << "=>" << x << endl
#else  /* DEBUG */
#   define debug( x )
#endif /* DEBUG */


static const char hexchars[]="0123456789abcdef"; /**< direct translation int -> hex */

/** \class AC_GDB
 * GDB protocol support
 */
class AC_GDB {
public:
  AC_GDB( AC_GDB_Interface* proc, int port );
  ~AC_GDB();

  void process_bp();
  bool stop( unsigned int decoded_pc );
  void exit( int ac_exit_status );

  /* Runtime Enable/Disable GDB Support */
  void disable();
  void enable();
  int  is_disabled();
  void connect();

  /* Set/Get Port */
  void set_port( int port );
  int  get_port();

private:
  Breakpoints *bps;       /**< Breakpoints */
  AC_GDB_Interface* proc; /**< Processor specific operations */

  /* Connection */
  int  sd;         /**< TCP communication socket */
  int  port;       /**< TCP communication port */

  /* Status */
  char connected;  /**< is connected? */
  char first_time; /**< is first time? */
  char step;       /**< is step mode? */
  char disabled;   /**< is GDB support disabled? */

  /* Buffers */
  char out_buffer[ GDB_BUFFERSIZE ]; /**< Output Buffer */
  char in_buffer[ GDB_BUFFERSIZE ];  /**< Input Buffer */

  /* Registers */
  void reg_read( char *ib, char *ob );
  void reg_write( char *ib, char *ob );

  /* Memory */
  void mem_read( char *ib, char *ob );
  void mem_write( char *ib, char *ob );

  /* Flow control */
  void continue_execution( char *ib, char *ob );
  void stepmode( char *ib, char *ob );
  void cc( char *ib, char *ob );

  /* Breakpoints */
  void break_insert( char *ib, char *ob );
  void break_remove( char *ib, char *ob );

  /* Communication */
  void comm_getpacket ( char *buffer );
  void comm_putpacket( const char *buffer );
  int  comm_putchar( const char c );
  char comm_getchar();

  /* Helpers */
  int  hex( unsigned char ch );
  ac_word changeendianess( ac_word src );
};

#endif /* _AC_GDB_H_ */

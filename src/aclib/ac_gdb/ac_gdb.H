/**
 * @file      ac_gdb.H
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
 *            This file implements AC_GDB methods to support GDB 
 *            communication protocol (see info gdb / "Remote Protocol"). 
 *            The current implementation supports the following gdb commands:
 \verbatim
 .----------------.---------------------------------------.-----------------.
 | Command        | Function Description                  | Return value    |
 }----------------+---------------------------------------+-----------------{
 | g              | Return the value of the CPU registers | hex data or ENN |
 | G              | Set the value of the CPU registers    | OK or ENN       |
 |                |                                       |                 |
 | mAA..AA,LLLL   | Read LLLL bytes at address AA..AA     | hex data or ENN |
 | MAA..AA,LLLL:  | Write LLLL bytes at address AA.AA     | OK or ENN       |
 |                |                                       |                 |
 | c              | Resume at current address             | SNN (signal NN) |
 | cAA..AA        | Continue at address AA..AA            | SNN             |
 |                |                                       |                 |
 | s              | Step one instruction                  | SNN             |
 | sAA..AA        | Step one instruction from AA..AA      | SNN             |
 |                |                                       |                 |
 | k              | kill                                  |                 |
 |                |                                       |                 |
 | ZT,AA..AA,LLLL | Insert breakpoint or watchpoint       | OK, ENN or ''   |
 | zT,AA..AA,LLLL | Remove breakpoint or watchpoint       | OK, ENN or ''   |
 |                |                                       |                 |
 | ?              | What was the last sigval ?            | SNN             |
 |                |                                       |                 |
 | 0x03           | Control-C                             |                 |
 `----------------'---------------------------------------'-----------------'
 \endverbatim
 *
 *    All commands and responses are sent with a packet which includes a
 * checksum.	A packet consists of
 *
 \verbatim
                 $<packet info>#<checksum>.
 \endverbatim
 *
 * where
 *   packet info = characters representing the command or response
 *   checksum    = two hex digits computed as modulo 256 sum of packet info
 *
 *    When a packet is received, it is first acknowledged with either '+' or '-'
 * '+' indicates a successful transfer.	 '-' indicates a failed transfer.
 *
 *    This file is to be processor agnostic!  Every code that depends on 
 * processor specific features must be handled in AC_GDB_Interface.
 *
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 * \note When modifing this file respect:
 * \li License
 * \li Previous author names. Add your own after current ones.
 * \li Coding style (basically emacs style)
 * \li Commenting style. This code use doxygen (http://www.doxygen.org)
 *     to be documented.
 *
 * \todo Right now, just memory breakpoints are supported, the other (hardware,
 *       write, read and access) are not implemented. They are marked as:
 *           \code // FIXME --- not yet supported \endcode
 *       If you want to improve GDB support, try to implement these.
 * NOTICE:
 *    Contains extracts from Linux Kernel code. See below.
 *
 */

#ifndef _AC_GDB_H_
#define _AC_GDB_H_

#include "breakpoints.H"
#include "ac_gdb_interface.H"

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
template <typename ac_word>
class AC_GDB {
public:
  AC_GDB( AC_GDB_Interface<ac_word>* proc, int port );
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
  AC_GDB_Interface<ac_word>* proc; /**< Processor specific operations */

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

/**
 * Constructor: create a GDB communication instance with the 
 * processor specific functions, implemented in \a proc. The simulator
 * will then listen for GDB connections at \a port
 *
 * \param proc processor specific functions that know how to read/write memory/registers.
 * \param port which port to listen for connections
 *
 * \return new AC_GDB instance
 */
template <typename ac_word>
AC_GDB<ac_word>::AC_GDB(AC_GDB_Interface<ac_word>* proc, int port) {
  this->connected  = 0;
  this->step       = 0;
  this->first_time = 1;
  this->proc       = proc;
  this->bps= new Breakpoints( BREAKPOINTS );
  this->set_port( port );
  this->disable();
}


/**
 * Destructor: Destroy AC_GDB instance
 */
template <typename ac_word>
AC_GDB<ac_word>::~AC_GDB() {
  delete bps;
  debug( "AC_GDB: connection closed!" );
}



/* Register Access ***********************************************************/

/**
 * Read registers to GDB.
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::reg_read(char *ib, char *ob) {
  int i;
  ob[ 0 ] = '\0';
  char fmt[ 5 ] = "";
  ac_word r;

  /* 1 byte is represented in hexadecimal by 2 chars */
  snprintf( fmt, 5, "%%%02dx", (int)sizeof( ac_word ) * 2 );

  for ( i = 0; i < proc->nRegs(); i ++ )
    {
      r = proc->reg_read( i );
      if (proc->get_ac_tgt_endian() == false)
        r = changeendianess(r);
      sprintf( ob + strlen( ob ), fmt, r );
    }
}


 /**
 * Write registers with GDB input.
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::reg_write(char *ib, char *ob) {
  ac_word aux;
  int i, ss;
  char fmt[ 5 ] = "";

  ib ++; /* skip G */

  /* 1 byte is represented in hexadecimal by 2 chars */
  ss = sizeof( ac_word ) * 2;
  snprintf( fmt, 5, "%%%02dx", ss );

  for ( i = 0; i < 32; i ++) {
    sscanf( ib, fmt, &aux );
    if (proc->get_ac_tgt_endian() == false)
      aux = changeendianess(aux);
    proc->reg_write( i, aux );
    ib += ss;
  }

  strncpy( ob, "OK", GDB_BUFFERSIZE );
}



/* Memory Access *************************************************************/

/**
 * Write simulator memory with data provided by GDB.
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::mem_write( char *ib, char *ob ) {
  unsigned i, r;
  unsigned address, bytes;
  char *ib_h = ib;
  char fmt[ 5 ] = "%02x";
  ac_word n;

  r = sscanf( ib, "M%x,%x:", &address, &bytes );

  ib = strchr( ib, ':' );

  if ( ( ! ib ) || ( r != 2 ) )
    /* Data is wrong! */
    strncpy( ob, "E01", GDB_BUFFERSIZE );
  else {
    int offset;
    ib ++; /* next char after ':' */

    /* be sure to not read outside in_buffer */
    offset = ib - ib_h;
    if ( ( ( bytes << 1 ) + 1 + offset ) >= GDB_BUFFERSIZE )
      /* [ offset | hex bytes repr...	 | \0 ] <= GDB_BUFFERSIZE */
      bytes = ( ( GDB_BUFFERSIZE - offset ) >> 1 ) - 1;


    for ( i = 0; i < (bytes * 2); i += 2 )
      {
	if ( ib[ i ] == 0 )
	  strncpy( ob, "E03", GDB_BUFFERSIZE); /* end of string ('\0'), this is an error! */
	n=0;
	sscanf( ib, fmt, &n );
	ib += 2;
	
	proc->mem_write(address,n);
	address++;

      }
    strncpy( ob, "OK", GDB_BUFFERSIZE );
  }
}


/**
 * Read simulator memory.
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::mem_read( char *ib, char *ob ) {
  unsigned i;
  unsigned address = 0, bytes = 0;
  char fmt[ 5 ] = "%02x";
  unsigned int n = 0;

  if ( sscanf( ib, "m%x,%x", &address, &bytes ) != 2 )
    /* Data is wrong! */
    strncpy( ob, "E01", GDB_BUFFERSIZE );
  else {
    /* be sure there is enough room for returning string + '\0' */
    if ( ( ( bytes << 1 ) + 1 ) >= GDB_BUFFERSIZE )
      /* Read just bytes that fit the buffer */
      bytes = ( GDB_BUFFERSIZE >> 1 ) - 1;

    for ( i = 0; i < bytes; i++ )
      {
	n = 0;
	n = proc->mem_read( address );
	address ++;

	snprintf( ob + i * 2, GDB_BUFFERSIZE, fmt, n );
      }

    ob[ i * 2 ] = '\0';

  }
}





/* Execution Control *********************************************************/

/**
 * Have simulator to continue executing
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::continue_execution( char *ib, char *ob ) {
  unsigned address;

  if ( sscanf( ib, "c%x", &address ) == 1 )
    proc->set_ac_pc(address);
  step = 0;
}


/**
 * Have simulator to go step by step
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::stepmode( char *ib, char *ob ) {
  unsigned address;
  if ( sscanf( ib, "s%x", &address ) == 1 )
    proc->set_ac_pc(address);

  step = 1;
}


/**
 * Control-C: SIGINT and return control to gdb
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::cc( char *ib, char *ob ) {
  snprintf( ob, GDB_BUFFERSIZE, "S%02x", SIGINT );
  step = 1;
}


/**
 * Send exit status to GDB.
 *
 * \param ac_exit_status exit status from process.
 */
template <typename ac_word>
void AC_GDB<ac_word>::exit(int ac_exit_status) {
  if ( disabled ) return;
  snprintf( out_buffer, GDB_BUFFERSIZE, "W%02x", ac_exit_status );
  comm_putpacket(out_buffer);
}


/* Break & Watch Point *******************************************************/

/**
 * Insert Break or Watch point.
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::break_insert( char *ib, char *ob ) {
  int type, address, length;

  if ( sscanf( ib, "Z%x,%x,%x", &type, &address, &length ) != 3 )
    /* Data is wrong! */
    strncpy( ob, "E01", GDB_BUFFERSIZE );
  else {
    switch ( type ) {
    case 0:
      /* memory breakpoint */
      if ( bps->add( address ) == 0 )
	strncpy( ob, "OK", GDB_BUFFERSIZE );
      else
	strncpy( ob, "E00", GDB_BUFFERSIZE );
      break;

    case 1:
      /* hardware breakpoint */
      ob[ 0 ] = 0; /* FIXME --- not yet supported */
      break;

    case 2:
      /* write watchpoint */
      ob[ 0 ] = 0; /* FIXME --- not yet supported */
      break;

    case 3:
      /* read watchpoint */
      ob[ 0 ] = 0; /* FIXME --- not yet supported */
      break;

    case 4:
      /* access watchpoint */
      ob[ 0 ] = 0; /* FIXME --- not yet supported */
      break;
    }
  }
}


/**
 * Remove Break or Watch Point.
 *
 * \param ib buffer with string received from GDB
 * \param ob buffer to store string to be sent to GDB
 */
template <typename ac_word>
void AC_GDB<ac_word>::break_remove( char *ib, char *ob ) {
  int type, address, length;

  if ( sscanf( ib, "z%x,%x,%x", &type, &address, &length ) != 3 )
    /* Data is wrong! */
    strncpy( ob, "E01", GDB_BUFFERSIZE );
  else {
    switch ( type )
      {
      case 0:
	/* memory breakpoint */
	if ( bps->remove( address ) == 0 )
	  strncpy( ob, "OK", GDB_BUFFERSIZE );
	else
	  strncpy( ob, "E00", GDB_BUFFERSIZE );
	break;

      case 1:
	/* hardware breakpoint */
	ob[ 0 ] = 0; /* FIXME --- not yet supported */
	break;

      case 2:
	/* write watchpoint */
	ob[ 0 ] = 0; /* FIXME --- not yet supported */
	break;

      case 3:
	/* read watchpoint */
	ob[ 0 ] = 0; /* FIXME --- not yet supported */
	break;

      case 4:
	/* access watchpoint */
	ob[ 0 ] = 0; /* FIXME --- not yet supported */
	break;
      }
  }
}





/* GDB Specific Functions: ***************************************************/

/**
 * Establish TCP Connection
 */
template <typename ac_word>
void AC_GDB<ac_word>::connect() {
  struct sockaddr_in cliAddr, servAddr;
  int sd;

  /* Create Socket */
  if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Cannot Open Socket");
    ::exit(127);
  }

  /* Bind To Server Port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(this->port);

  { /* reuse address */
    int yes=1;
    if ( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror( "setsockopt" );
      ::exit(127);
    }
  }

  fprintf(stderr, "AC_GDB: listening to port %d\n", this->port);
  if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
    perror("bind");
    ::exit(127);
  }

  listen(sd,5);

  /* Listen for a connection */
  {
    socklen_t cliLen;
    cliLen = sizeof(cliAddr);

    if ((this->sd = accept(sd, (struct sockaddr *) &cliAddr, &cliLen)) < 0 ) {
      perror("accept");
      ::exit(127);
    }
  }

  connected = 1;
  fprintf(stderr, "AC_GDB: connected to port %d\n", this->port);
}


/*******************************************************************************
 * Process break point.                                                        *
 ******************************************************************************/

/**
 *    Return if the processor must stop or not. It must stop if it's the first 
 * time, it's in step mode or there's a breakpoint for that address.
 *
 * \param decoded_pc decoded program counter (PC, current address).
 *
 * \return true if it must stop, false otherwise.
 */
template <typename ac_word>
bool AC_GDB<ac_word>::stop(unsigned int decoded_pc) {
  if ( disabled ) return false;
  
  if ( first_time || step || bps->exists(decoded_pc))
    return true;
  return false;
}


/**
 * Process the next packet from gdb and take the needed action.
 */
template <typename ac_word>
void AC_GDB<ac_word>::process_bp() {
  if ( disabled ) return;
  first_time=0;
  
  snprintf( out_buffer, GDB_BUFFERSIZE, "S%02x", SIGTRAP );
  comm_putpacket(out_buffer);
  
  if ( ! connected ) return;

  while (1) {

    out_buffer[0] = 0;

    comm_getpacket(in_buffer);

    switch (in_buffer[0]) {
    case '?':
      /* "?": Return the reason simulator halted */
      snprintf( out_buffer, GDB_BUFFERSIZE, "S%02x", SIGTRAP );
      break;

    case 'g':
      /* "g": return the value of the CPU registers as hex data or ENN */
      reg_read( in_buffer, out_buffer );
      break;

    case 'G':
      /* "G": set the value of the CPU registers. */
      reg_write( in_buffer, out_buffer );
      break;

    case 'm':
      /* "mAA..AA,LLLL": Read LLLL bytes at address AA..AA as hex data or ENN*/
      mem_read( in_buffer, out_buffer );
      break;

    case 'M':
      /* "MAA..AA,LLLL:": Write LLLL bytes at address AA.AA return OK */
      mem_write( in_buffer, out_buffer );
      break;

    case 'c':
      /* "cAA..AA": continue at address AA..AA or same address if no AA..AA*/
      continue_execution( in_buffer, out_buffer );
      return;

    case 's':
      /* "sAA..AA": resume at address AA..AA or same address if no AA..AA */
      stepmode( in_buffer, out_buffer );
      return;

    case 0x03:
      /* Control-C: return control to gdb */
      cc( in_buffer, out_buffer );
      comm_putpacket( out_buffer );
      return;

    case 'k' :
      /* "k": kill */
      proc->ac_stop();
      break;

    case 'Z':
      /* "ZT,AA..AA,LLLL": Insert breakpoint or watchpoint */
      break_insert( in_buffer, out_buffer );
      break;

    case 'z':
      /* "zT,AA..AA,LLLL": Remove breakpoint or watchpoint */
      break_remove( in_buffer, out_buffer );
      break;

    default:
      out_buffer[0] = 0; /* Null response */
    }

    comm_putpacket(out_buffer);
  }
}









/* Helper Functions: *********************************************************/

/* Copyright notice:
 *
 *		Most of this code was taken from: arch/mips/kernel/gdb-stub.c
 *
 * Original copyright:
 *
 *	Originally written by Glenn Engel, Lake Stevens Instrument Division
 *
 *	Contributed by HP Systems
 *
 *	Modified for SPARC by Stu Grossman, Cygnus Support.
 *
 *	Modified for Linux/MIPS (and MIPS in general) by Andreas Busse
 *	Send complaints, suggestions etc. to <andy@waldorf-gmbh.de>
 *
 *	Copyright (C) 1995 Andreas Busse
 *
 *	Copyright (C) 2003 MontaVista Software Inc.
 *	Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 */

/**
 *	Convert from a hex digit to an int
 *
 * \param ch hexadecimal digit
 *
 * \return corresponding integer
 */
template <typename ac_word>
int AC_GDB<ac_word>::hex(unsigned char ch)
{
  if (ch >= 'a' && ch <= 'f')
    return ch-'a'+10;
  if (ch >= '0' && ch <= '9')
    return ch-'0';
  if (ch >= 'A' && ch <= 'F')
    return ch-'A'+10;
  return -1;
}



/**
 * Change word endianess.
 *
 * \param src source word
 *
 * \return endianess swapped word
 */
template <typename ac_word>
ac_word AC_GDB<ac_word>::changeendianess( ac_word src )
{
  ac_word dst = 0;
  int i, j;
  int size = sizeof( src );
  
  for ( i=0, j=( size - 1 );
	i < size;
	i++, j-- )
    dst |= ( ( src >> ( i << 3 ) ) & 0xff ) << ( j << 3 );
  
  return dst;
}


/**
 * scan for the sequence $<data>#<checksum>
 *
 * \param buffer buffer to receive the packet.
 */
template <typename ac_word>
void AC_GDB<ac_word>::comm_getpacket (char *buffer) {
  unsigned char checksum;
  unsigned char xmitcsum;
  int i;
  int count;
  unsigned char ch;

  do {
    /*
     * wait around for the start character,
     * ignore all other characters
     */
    while ( ( ch = ( comm_getchar() & 0x7f ) ) != '$' );

    checksum = 0;
    xmitcsum = 255;
    count = 0;

    /*
     * now, read until a # or end of buffer is found
     */
    while ( count < GDB_BUFFERSIZE )
      {
	ch = comm_getchar();
	if ( ch == '#' )
	  break;
	checksum = checksum + ch;
	buffer[ count ] = ch;
	count = count + 1;
      }

    if ( count >= GDB_BUFFERSIZE )
      continue;

    buffer[ count ] = 0;

    if ( ch == '#' )
      {
	xmitcsum	= hex( comm_getchar() & 0x7f ) << 4;
	xmitcsum |= hex( comm_getchar() & 0x7f );

	if ( checksum != xmitcsum )
	  comm_putchar( '-' ); /* failed checksum */
	else {
	  comm_putchar( '+' ); /* successful transfer */

	  /*
	   * if a sequence char is present,
	   * reply the sequence ID
	   */
	  if ( buffer[ 2 ] == ':' )
	    {
	      comm_putchar( buffer[0] );
	      comm_putchar( buffer[1] );

	      /*
	       * remove sequence chars from buffer
	       */
	      count = strlen( buffer );
	      for ( i = 3; i <= count; i ++ )
		buffer[ i - 3] = buffer[ i ];
	    }
	}
      }
  }
  while ( checksum != xmitcsum );

  debug("received packet:" << buffer);
}



/**
 * send the packet in buffer.
 *
 * \param buffer string to be sent.
 */
template <typename ac_word>
void AC_GDB<ac_word>::comm_putpacket(const char *buffer) {
  unsigned char checksum;
  int count;
  unsigned char ch;

  debug("out packet:" << buffer << endl);

  /*
   * $<packet info>#<checksum>.
   */

  do
    {
      comm_putchar( '$' );
      checksum = 0;
      count		 = 0;

      while ( ( ch = buffer[ count ] ) != 0 )
	{
	  if ( ! ( comm_putchar( ch ) ) )
	    return;
	  checksum += ch;
	  count		 += 1;
	}

      comm_putchar( '#' );
      comm_putchar( hexchars[ checksum >> 4 ] );
      comm_putchar( hexchars[ checksum & 0xf ] );

    }
  while ( ( comm_getchar() & 0x7f) != '+' );
}


/**
 * Put char (byte) to output queue.
 *
 * \param c char to be sent.
 */
template <typename ac_word>
int AC_GDB<ac_word>::comm_putchar(const char c) {
  return write( sd, &c, 1 );
}


/**
 * Get char (byte) from input queue.
 *
 * \return char from input queue.
 */
template <typename ac_word>
char AC_GDB<ac_word>::comm_getchar() {
  char c;
  if (read( sd, &c, 1 ) != 1)
    return 0; /* Error! */
  return(c);
}



/**
 * Disable GDB support
 */
template <typename ac_word>
void AC_GDB<ac_word>::disable() {
  this->disabled = 1;
}

/**
 * Enable GDB support
 */
template <typename ac_word>
void AC_GDB<ac_word>::enable() {
  this->disabled = 0;
}



/**
 * Return if GDB Support is disabled or not
 *
 * \return 1 if it's disabled, 0 otherwise;
 */
template <typename ac_word>
int AC_GDB<ac_word>::is_disabled() {
  return (int) this->disabled;
}



/**
 * Set GDB port. Just have effect on next AC_GDB::connect().
 *
 * \param port new port
 */
template <typename ac_word>
void AC_GDB<ac_word>::set_port( int port ) {
  this->port = port;
}

/**
 * Get GDB port.
 *
 * \return current gdb port
 */
template <typename ac_word>
int AC_GDB<ac_word>::get_port() {
  return this->port;
}

#endif /* _AC_GDB_H_ */

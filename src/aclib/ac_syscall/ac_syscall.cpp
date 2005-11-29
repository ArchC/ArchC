#include "ac_syscall.H"
#include "ac_utils.H"
#include "ac_arch.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/times.h>
#include <time.h>

// Fix incompatibility from NewLib flags and Linux flags

#define NEWLIB_O_RDONLY          0x0000
#define NEWLIB_O_WRONLY          0x0001
#define NEWLIB_O_RDWR            0x0002
#define NEWLIB_O_APPEND          0x0008
#define NEWLIB_O_CREAT           0x0200
#define NEWLIB_O_TRUNC           0x0400
#define NEWLIB_O_EXCL            0x0800
#define NEWLIB_O_NOCTTY          0x8000
#define NEWLIB_O_NONBLOCK        0x4000

#define CORRECT_O_RDONLY             00
#define CORRECT_O_WRONLY             01
#define CORRECT_O_RDWR               02
#define CORRECT_O_CREAT            0100
#define CORRECT_O_EXCL             0200
#define CORRECT_O_NOCTTY           0400
#define CORRECT_O_TRUNC           01000
#define CORRECT_O_APPEND          02000
#define CORRECT_O_NONBLOCK        04000

void correct_flags( int* val )
{
  int f = *val;
  int flags = 0;

  if( f &  NEWLIB_O_RDONLY )
    flags |= CORRECT_O_RDONLY;
  if( f &  NEWLIB_O_WRONLY )
    flags |= CORRECT_O_WRONLY;
  if( f &  NEWLIB_O_RDWR )
    flags |= CORRECT_O_RDWR;
  if( f & NEWLIB_O_CREAT )
    flags |= CORRECT_O_CREAT;
  if( f & NEWLIB_O_EXCL )
    flags |= CORRECT_O_EXCL;
  if( f & NEWLIB_O_NOCTTY )
    flags |= CORRECT_O_NOCTTY;
  if( f & NEWLIB_O_TRUNC )
    flags |= CORRECT_O_TRUNC;
  if( f & NEWLIB_O_APPEND )
    flags |= CORRECT_O_APPEND;
  if( f & NEWLIB_O_NONBLOCK )
    flags |= CORRECT_O_NONBLOCK;

  *val = flags;
}

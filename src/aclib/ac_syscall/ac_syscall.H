/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  ArchC Syscall Library for the ArchC architecture simulators
    Copyright (C) 2002-2004  The ArchC Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

/********************************************************/
/* ArchC Syscalls header file.                          */
/* Author:  Marcus Bartholomeu                          */
/*                                                      */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

#ifndef AC_SYSCALL_H
#define AC_SYSCALL_H

#ifdef USE_GDB
#include "ac_gdb.H"
#endif /* USE_GDB */

#include "ac_arch_ref.H"

template <class ac_word, class ac_Hword> class ac_syscall {
private:
  ac_arch<ac_word, ac_Hword>& ref;

public:
//   ac_arch_ref ref;

  ac_syscall(ac_arch<ac_word, ac_Hword>& r) : ref(r) {};

#define AC_SYSC(NAME,LOCATION) \
  void NAME();

#include <ac_syscall.def>

#undef AC_SYSC

  //!Target dependent functions
  virtual void get_buffer(int argn, unsigned char* buf, unsigned int size) =0;
  virtual void set_buffer(int argn, unsigned char* buf, unsigned int size) =0;
  virtual int  get_int(int argn) =0;
  virtual void set_int(int argn, int val) =0;
  virtual void return_from_syscall() =0;
  virtual void set_prog_args(int argc, char *argv[]) =0;
};

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

#undef AC_RUN_ERROR
#define AC_RUN_ERROR( str, args...) \
  fprintf(stderr, "ArchC Runtime error (ac_pc=%#x; ac_instr_counter=%llu): " str, (int) ref.ac_pc, ref.ac_instr_counter , ##args)


#ifdef DEBUG
#  define DEBUG_SYSCALL(name) AC_RUN_MSG("@@@@@ syscall: " name " @@@@@\n")
#else
#  define DEBUG_SYSCALL(name)
#endif

void correct_flags( int* val );

//! Processor independent functions (syscalls)

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::open()
{
  DEBUG_SYSCALL("open");
  unsigned char pathname[100];
  get_buffer(0, pathname, 100);
  int flags = get_int(1); correct_flags(&flags);
  int mode = get_int(2);
  int ret = ::open((char*)pathname, flags, mode);
  if (ret == -1) {
    AC_RUN_ERROR("System Call open (file '%s'): %s\n", pathname, strerror(errno));
    exit(EXIT_FAILURE);
  }
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::creat()
{
  DEBUG_SYSCALL("creat");
  unsigned char pathname[100];
  get_buffer(0, pathname, 100);
  int mode = get_int(1);
  int ret = ::creat((char*)pathname, mode);
  if (ret == -1) {
    AC_RUN_ERROR("System Call creat (file '%s'): %s\n", pathname,strerror(errno));
    exit(EXIT_FAILURE);
  }
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::close()
{
  DEBUG_SYSCALL("close");
  int fd = get_int(0);
  int ret = ::close(fd);
  if (ret == -1) {
    AC_RUN_ERROR("System Call close (fd %d): %s\n", fd, strerror(errno));
    exit(EXIT_FAILURE);
  }
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::read()
{
  DEBUG_SYSCALL("read");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  int ret = ::read(fd, buf, count);
  if (ret == -1) {
    AC_RUN_ERROR("System Call read (fd %d): %s\n", fd, strerror(errno));
    exit(EXIT_FAILURE);
  }
  set_buffer(1, buf, ret);
  set_int(0, ret);
  return_from_syscall();
  free(buf);
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::write()
{
  DEBUG_SYSCALL("write");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  get_buffer(1, buf, count);
  int ret = ::write(fd, buf, count);
  if (ret == -1) {
    AC_RUN_ERROR("System Call write (fd %d): %s\n", fd, strerror(errno));
    exit(EXIT_FAILURE);
  }
  set_int(0, ret);
  return_from_syscall();
  free(buf);
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::isatty()
{
  DEBUG_SYSCALL("isatty");
  int desc = get_int(0);
  int ret = ::isatty(desc);
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::sbrk()
{
  DEBUG_SYSCALL("sbrk");
  int base;
//   extern unsigned int ac_heap_ptr;

  int increment = get_int(0);
  if ((ref.IM->get_size() - ref.ac_heap_ptr - increment) >= 0) {
    base = ref.ac_heap_ptr;
    (ref.ac_heap_ptr) += increment;
  }
  else {
    AC_RUN_ERROR("System Call sbrk: Not enough memory left in the virtual machine to allocate %d bytes (ram size=%u, heap=%u)\n", increment, ref.IM->get_size(), ref.ac_heap_ptr);
    exit(EXIT_FAILURE);
  }

  set_int(0, base);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::lseek()
{
  DEBUG_SYSCALL("lseek");
  int fd = get_int(0);
  int offset = get_int(1);
  int whence = get_int(2);
  int ret = ::lseek(fd, offset, whence);
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::fstat()
{
  DEBUG_SYSCALL("fstat");
  int fd = get_int(0);
  struct stat buf;
  int ret = ::fstat(fd, &buf);
  if (ret == -1) {
    AC_RUN_ERROR("System Call fstat (fd %d): %s\n", fd, strerror(errno));
    exit(EXIT_FAILURE);
  }
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::_exit()
{
  DEBUG_SYSCALL("_exit");
  int ac_exit_status = get_int(0);
#ifdef USE_GDB
  if (ref.get_gdbstub()) (ref.get_gdbstub())->exit(ac_exit_status);
#endif /* USE_GDB */
  ref.stop(ac_exit_status);
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::times()
{
  DEBUG_SYSCALL("times");
  unsigned char zeros[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  set_buffer(0, zeros, 16);
  set_int(0, 0);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::time()
{
  DEBUG_SYSCALL("time");
  int t = get_int(0);
  int ret = ::time(0);
  if (t!=0) set_buffer(0, (unsigned char *) &ret, 4);
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::random()
{
  DEBUG_SYSCALL("random");
  int ret = ::random();
  set_int(0, ret);
  return_from_syscall();
}


#include <ac_syscall_codes.h>

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::ac_syscall_wrapper()
{
  int ret = -1;
  unsigned char pathname[100];
  int mode;
  int oldfd, newfd;

  int syscall_code = get_int(0);

  switch(syscall_code) {

  case __NR_getpid:
    DEBUG_SYSCALL("getpid");
    ret = 123;
    break;

  case __NR_chmod:
    DEBUG_SYSCALL("chmod");
    get_buffer(0, pathname, 100);
    mode = get_int(1);
    ret = ::chmod((char*)pathname, mode);
    break;

  case __NR_dup:
    DEBUG_SYSCALL("dup");
    oldfd = get_int(1);
    ret = ::dup(oldfd);
    break;

  case __NR_dup2:
    DEBUG_SYSCALL("dup2");
    oldfd = get_int(1);
    newfd = get_int(2);
    ret = ::dup2(oldfd, newfd);
    break;

  default:
    AC_RUN_ERROR("System Call code %d not implemented yet.\n", syscall_code);
    exit(EXIT_FAILURE);
  }
    
  //if (ret == -1)   AC_RUN_ERROR("System call %d returned -1.\n", syscall_code);
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::ac_syscall_geterrno()
{
  set_int(0, errno);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::ac_syscall_stat_mode()
{
  AC_RUN_ERROR("System Call ac_syscall_stat_mode not implemented yet.\n");
  exit(EXIT_FAILURE);
}

#endif

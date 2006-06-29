/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_syscall.H
 * @author    Marcus Bartholomeu
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
 *
 * @brief     ArchC Syscalls header file.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef AC_SYSCALL_H
#define AC_SYSCALL_H

#ifdef USE_GDB
#include "ac_gdb.H"
#endif /* USE_GDB */

#include "ac_arch_ref.H"
#include "ac_utils.H"

template <class ac_word, class ac_Hword> class ac_syscall {
private:
  ac_arch<ac_word, ac_Hword>& ref;
  const unsigned int ramsize;

public:
  ac_syscall(ac_arch<ac_word, ac_Hword>& r, unsigned int rs) : ref(r), ramsize(rs) {};

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

#include <iostream>
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
#if 0 /// Changed to iostream-type. --Marilia
#define AC_RUN_ERROR( str, args...) \
  fprintf(stderr, "ArchC Runtime error (ac_pc=%#x; ac_instr_counter=%llu): " str, (int) ref.get_ac_pc(), ref.ac_instr_counter , ##args)
#else
#define AC_RUN_ERROR \
  std::cerr << "ArchC Runtime error (ac_pc=" << std::hex << ref.get_ac_pc() << std::dec << "; ac_instr_counter=" << ref.ac_instr_counter << "): "
#endif

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
#ifdef AC_MEM_HIERARCHY
  if (!flush_cache()) return;
#endif
  DEBUG_SYSCALL("open");
  unsigned char pathname[100];
  get_buffer(0, pathname, 100);
  int flags = get_int(1); correct_flags(&flags);
  int mode = get_int(2);
  int ret = ::open((char*)pathname, flags, mode);
  if (ret == -1) {
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call open (file '%s'): %s\n", pathname, strerror(errno));
#else
    AC_RUN_ERROR << "System Call open (file '" << pathname << "'): " << strerror(errno) << std::endl;
#endif
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
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call creat (file '%s'): %s\n", pathname, strerror(errno));
#else
    AC_RUN_ERROR << "System Call creat (file '" << pathname << "'): " << strerror(errno) << std::endl;
#endif
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
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call close (fd %d): %s\n", fd, strerror(errno));
#else
    AC_RUN_ERROR << "System Call close (fd " << fd << "): " << strerror(errno) << std::endl;
#endif
    exit(EXIT_FAILURE);
  }
  set_int(0, ret);
  return_from_syscall();
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::read()
{
#ifdef AC_MEM_HIERARCHY
  if (!flush_cache()) return;
#endif  
  DEBUG_SYSCALL("read");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  int ret = ::read(fd, buf, count);
  if (ret == -1) {
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call read (fd %d): %s\n", fd, strerror(errno));
#else
    AC_RUN_ERROR << "System Call read (fd " << fd << "): " << strerror(errno) << std::endl;
#endif
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
#ifdef AC_MEM_HIERARCHY
  if (!flush_cache()) return;
#endif
  DEBUG_SYSCALL("write");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  get_buffer(1, buf, count);
  int ret = ::write(fd, buf, count);
  if (ret == -1) {
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call write (fd %d): %s\n", fd, strerror(errno));
#else
    AC_RUN_ERROR << "System Call write (fd " << fd << "): " << strerror(errno) << std::endl;
#endif
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
  unsigned int base = ref.ac_heap_ptr;
  unsigned int increment = get_int(0);
  ref.ac_heap_ptr += increment;

  // Test if there is enough space in the target memory 
  // OBS: 1kb is reserved at the end of memory to command line parameters
  if (ref.ac_heap_ptr > ramsize-1024) {
    // Show error only once
    static bool show_error = true;
    if (show_error) {
      AC_WARN("Target application failed to allocate " << increment <<
               " bytes: heap(=" << ref.ac_heap_ptr << ") > ramsize(=" <<
               ramsize << ")");
      AC_WARN("If target application does not treat allocation error, it may crash.");
    }
    show_error = false;
    ref.ac_heap_ptr = base;
    set_int(0, -1);
  }
  else {
    set_int(0, base);
  }

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
  static bool fstat_warn = true;
  if (fstat_warn) {
    AC_WARN("This version of fstat should not be called!");
    AC_WARN("Please, recompile your target application with an updated libac_sysc.");
    fstat_warn = false;
  }
  int fd = get_int(0);
  struct stat buf;
  int ret = ::fstat(fd, &buf);
  if (ret == -1) {
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call fstat (fd %d): %s\n", fd, strerror(errno));
#else
    AC_RUN_ERROR << "System Call fstat (fd " << fd << "): " << strerror(errno) << std::endl;
#endif
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
  int fd, newfd;
  static struct stat buf_stat;

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
    fd = get_int(1);
    ret = ::dup(fd);
    break;

  case __NR_dup2:
    DEBUG_SYSCALL("dup2");
    fd = get_int(1);
    newfd = get_int(2);
    ret = ::dup2(fd, newfd);
    break;

  case __NR_fstat:
    DEBUG_SYSCALL("fstat");
    fd = get_int(1);
    ret = ::fstat(fd, &buf_stat);
    break;


    /* Special cases for the fields of the "struct stat":
       to convert from glibc to newlib */

#define FILL_STRUCT_STAT(x) \
  case __AC_struct_stat_##x: \
    DEBUG_SYSCALL("filling struct stat field: " #x); \
    ret = buf_stat.x; \
    break

    FILL_STRUCT_STAT(st_dev);
    FILL_STRUCT_STAT(st_ino);
    FILL_STRUCT_STAT(st_mode);
    FILL_STRUCT_STAT(st_nlink);
    FILL_STRUCT_STAT(st_uid);
    FILL_STRUCT_STAT(st_gid);
    FILL_STRUCT_STAT(st_rdev);
    FILL_STRUCT_STAT(st_size);
    FILL_STRUCT_STAT(st_blksize);
    FILL_STRUCT_STAT(st_blocks);
    FILL_STRUCT_STAT(st_atime);
    FILL_STRUCT_STAT(st_mtime);
    FILL_STRUCT_STAT(st_ctime);

#undef FILL_STRUCT_STAT


  default:
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call code %d not implemented yet.\n", syscall_code);
#else
    AC_RUN_ERROR << "System Call code " << syscall_code << " not implemented yet." << std::endl;
#endif
    exit(EXIT_FAILURE);
  }
    
#if 0 /// Changed to iostream-type. --Marilia
  //if (ret == -1)   AC_RUN_ERROR("System call %d returned -1.\n", syscall_code);
#else
  //if (ret == -1)   AC_RUN_ERROR << "System call " << syscall_code << " returned -1." << std::endl;
#endif
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
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call ac_syscall_stat_mode not implemented yet.\n");
#else
    AC_RUN_ERROR << "System Call ac_syscall_stat_mode not implemented yet." << std::endl;
#endif
  exit(EXIT_FAILURE);
}

#endif

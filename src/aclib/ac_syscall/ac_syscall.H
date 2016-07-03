/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_syscall.H
 * @author    Marcus Bartholomeu
 *            Rafael Auler
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
 *
 * @Modified  08-2010
 * Static Compiled Simulator compatibility  
 */

#ifndef AC_SYSCALL_H
#define AC_SYSCALL_H

#ifndef AC_COMPSIM

#ifdef USE_GDB
#include "ac_gdb.H"
#endif /* USE_GDB */

#include "ac_rtld.H"
#include "ac_arch_ref.H"
#include "ac_utils.H"

template <class ac_word, class ac_Hword> class ac_syscall {
protected:
  ac_arch<ac_word, ac_Hword>& ref;
  const unsigned int ramsize;

public:
  ac_syscall(ac_arch<ac_word, ac_Hword>& r, unsigned int rs) : ref(r), ramsize(rs) {};

#define AC_SYSC(NAME,LOCATION) \
  void NAME();

#include <ac_syscall.def>

#undef AC_SYSC

  int process_syscall(int syscall);

  //!Target dependent functions
  virtual void get_buffer(int argn, unsigned char* buf, unsigned int size) =0;
  virtual void guest2hostmemcpy(unsigned char *dst, uint32_t src,
                                unsigned int size);
  virtual void set_buffer(int argn, unsigned char *buf, unsigned int size) = 0;
  virtual void host2guestmemcpy(uint32_t dst, unsigned char *src,
                                unsigned int size);
  virtual int get_int(int argn) = 0;
  virtual void set_int(int argn, int val) =0;
  virtual void return_from_syscall() =0;
  virtual void set_prog_args(int argc, char *argv[]) =0;
  virtual void set_pc(unsigned val);
  virtual void set_return(unsigned val);
  virtual unsigned get_return();
  virtual bool is_mmap_anonymous(uint32_t flags);
  virtual uint32_t convert_open_flags(uint32_t flags);
  virtual int *get_syscall_table();
};

#include "ac_utils.H"
#include "ac_arch.H"

#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/times.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

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

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::set_pc(unsigned val) {
  AC_RUN_ERROR << "You must implement set_pc() in your model syscall module."
               << std::endl;
  exit(EXIT_FAILURE);
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::set_return(unsigned val) {
  AC_RUN_ERROR << "You must implement set_return() in your model syscall module."
               << std::endl;
  exit(EXIT_FAILURE);
}
 
template <class ac_word, class ac_Hword>
unsigned ac_syscall<ac_word, ac_Hword>::get_return() {
  AC_RUN_ERROR << "You must implement get_return() in your model syscall module."
               << std::endl;
  exit(EXIT_FAILURE);
}

template <class ac_word, class ac_Hword>
bool ac_syscall<ac_word, ac_Hword>::is_mmap_anonymous(uint32_t flags) {
  AC_RUN_ERROR << "You must implement is_mmap_anonymous() in your model syscall module."
               << std::endl;
  exit(EXIT_FAILURE);
}

template <class ac_word, class ac_Hword>
uint32_t ac_syscall<ac_word, ac_Hword>::convert_open_flags(uint32_t flags) {
  return flags;
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::guest2hostmemcpy(unsigned char *dst,
                                                     uint32_t src,
                                                     unsigned int size) {
  AC_RUN_ERROR
      << "You must implement guest2hostmemcpy() in your model syscall module."
      << std::endl;
  exit(EXIT_FAILURE);
}

template <class ac_word, class ac_Hword>
void ac_syscall<ac_word, ac_Hword>::host2guestmemcpy(uint32_t dst,
                                                     unsigned char *src,
                                                     unsigned int size) {
  AC_RUN_ERROR
      << "You must implement host2guestmemcpy() in your model syscall module."
      << std::endl;
  exit(EXIT_FAILURE);
}

template <class ac_word, class ac_Hword>
int * ac_syscall<ac_word, ac_Hword>::get_syscall_table() {
  return NULL;
}

#endif // ifndef AC_COMPSIM

#ifndef AC_COMPSIM
#define AC_SYSCALL template <class ac_word, class ac_Hword> void ac_syscall<ac_word, ac_Hword>
#endif

/*                                                     *
 *  The macro AC_SYSCALL is necessary to maintain      *
 *  compatibility with Static Compiled Simulator       *  
 *                                                     */              

//! Processor independent functions (syscalls)

AC_SYSCALL::open()
{
/*#ifdef AC_MEM_HIERARCHY
  if (!flush_cache()) return;
#endif*/
  DEBUG_SYSCALL("open");
  unsigned char pathname[100];
  get_buffer(0, pathname, 100);
  int flags = get_int(1); correct_flags(&flags);
  int mode = get_int(2);
  int ret = ::open((char*)pathname, flags, mode);
//  if (ret == -1) {
//#if 0 /// Changed to iostream-type. --Marilia
//    AC_RUN_ERROR("System Call open (file '%s'): %s\n", pathname, strerror(errno));
//#else
//    AC_RUN_ERROR << "System Call open (file '" << pathname << "'): " << strerror(errno) << std::endl;
//#endif
//    exit(EXIT_FAILURE);
//  }
  set_int(0, ret);
  return_from_syscall();
}

AC_SYSCALL::creat()
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

AC_SYSCALL::close()
{
  DEBUG_SYSCALL("close");
  int fd = get_int(0);
  int ret;
  // Silently ignore attempts to close standard streams (newlib may try to do so when exiting)
  if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
    ret = 0;
  else
    ret = ::close(fd);
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

AC_SYSCALL::read()
{
/*#ifdef AC_MEM_HIERARCHY
  if (!flush_cache()) return;
#endif  */
  DEBUG_SYSCALL("read");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  int ret = ::read(fd, buf, count);
//  if (ret == -1) {
//#if 0 /// Changed to iostream-type. --Marilia
//    AC_RUN_ERROR("System Call read (fd %d): %s\n", fd, strerror(errno));
//#else
//    AC_RUN_ERROR << "System Call read (fd " << fd << "): " << strerror(errno) << std::endl;
//#endif
//    exit(EXIT_FAILURE);
//  }
  set_buffer(1, buf, ret);
  set_int(0, ret);
  return_from_syscall();
  free(buf);
}

AC_SYSCALL::write()
{
/*#ifdef AC_MEM_HIERARCHY
  if (!flush_cache()) return;
#endif*/
  DEBUG_SYSCALL("write");
  int fd = get_int(0);
  unsigned count = get_int(2);
  unsigned char *buf = (unsigned char*) malloc(count);
  get_buffer(1, buf, count);
  int ret = ::write(fd, buf, count);
//  if (ret == -1) {
//#if 0 /// Changed to iostream-type. --Marilia
//    AC_RUN_ERROR("System Call write (fd %d): %s\n", fd, strerror(errno));
//#else
//    AC_RUN_ERROR << "System Call write (fd " << fd << "): " << strerror(errno) << std::endl;
//#endif
//    exit(EXIT_FAILURE);
//  }
  set_int(0, ret);
  return_from_syscall();
  free(buf);
}

AC_SYSCALL::isatty()
{
  DEBUG_SYSCALL("isatty");
  int desc = get_int(0);
  int ret = ::isatty(desc);
  set_int(0, ret);
  return_from_syscall();
}

AC_SYSCALL::sbrk()
{
#ifndef AC_COMPSIM
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

#else

  DEBUG_SYSCALL("sbrk");
  unsigned int base = ac_heap_ptr;
  unsigned int increment = get_int(0);
  ac_heap_ptr += increment;

  // Test if there is enough space in the target memory 
  // OBS: 1kb is reserved at the end of memory to command line parameters
  if (ac_heap_ptr > AC_RAMSIZE-1024) {
    // Show error only once
    static bool show_error = true;
    if (show_error) {
      AC_WARN("Target application failed to allocate " << increment <<
               " bytes: heap(=" << ac_heap_ptr << ") > ramsize(=" <<
               AC_RAMSIZE << ")");
      AC_WARN("If target application does not treat allocation error, it may crash.");
    }
    show_error = false;
    ac_heap_ptr = base;
    set_int(0, -1);
  }
  else {
    set_int(0, base);
  }
#endif

  return_from_syscall();
}

AC_SYSCALL::lseek()
{
  DEBUG_SYSCALL("lseek");
  int fd = get_int(0);
  int offset = get_int(1);
  int whence = get_int(2);
  int ret = ::lseek(fd, offset, whence);
  set_int(0, ret);
  return_from_syscall();
}

//AC_SYSCALL::fstat()
//{
//  DEBUG_SYSCALL("fstat");
//  static bool fstat_warn = true;
//  if (fstat_warn) {
//    AC_WARN("This version of fstat should not be called!");
//    AC_WARN("Please, recompile your target application with an updated libac_sysc.");
//    fstat_warn = false;
//  }
//  int fd = get_int(0);
//  struct stat buf;
//  int ret = ::fstat(fd, &buf);
//  if (ret == -1) {
//#if 0 /// Changed to iostream-type. --Marilia
//    AC_RUN_ERROR("System Call fstat (fd %d): %s\n", fd, strerror(errno));
//#else
//    AC_RUN_ERROR << "System Call fstat (fd " << fd << "): " << strerror(errno) << std::endl;
//#endif
//    exit(EXIT_FAILURE);
//  }
//  set_int(0, ret);
//  return_from_syscall();
//}

AC_SYSCALL::_exit()
{
#ifndef AC_COMPSIM

  DEBUG_SYSCALL("_exit");
  int ac_exit_status = get_int(0);
#ifdef USE_GDB
  if (ref.get_gdbstub()) (ref.get_gdbstub())->exit(ac_exit_status);
#endif /* USE_GDB */
  ref.stop(ac_exit_status);

#else

  DEBUG_SYSCALL("_exit");
  int ac_exit_status = get_int(0);
#ifdef USE_GDB
  if (get_gdbstub()) (get_gdbstub())->exit(ac_exit_status);
#endif /* USE_GDB */
  stop(ac_exit_status);

#endif
}

AC_SYSCALL::times()
{
  DEBUG_SYSCALL("times");
  unsigned char zeros[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  set_buffer(0, zeros, 16);
  set_int(0, 0);
  return_from_syscall();
}

AC_SYSCALL::time()
{
  DEBUG_SYSCALL("time");
  int t = get_int(0);
  int ret = ::time(0);
  if (t!=0) set_buffer(0, (unsigned char *) &ret, 4);
  set_int(0, ret);
  return_from_syscall();
}

AC_SYSCALL::random()
{
  DEBUG_SYSCALL("random");
  int ret = ::random();
  set_int(0, ret);
  return_from_syscall();
}


AC_SYSCALL::getcwd()
{
  DEBUG_SYSCALL("getcwd");
  unsigned size = get_int(1);
  char *buf = (char*) malloc(size);
  long ret = (long) ::getcwd(buf, size);
  set_buffer(0, (unsigned char*) buf, ret);
  set_int(0, ret);
  return_from_syscall();
  free(buf);
}

AC_SYSCALL::getpagesize()
{
  DEBUG_SYSCALL("getpagesize");

  //FIXME: page size changes depending the architecture
  int ret = sysconf(_SC_PAGE_SIZE);

  set_int(0, ret);
  return_from_syscall();
}

AC_SYSCALL::execve()
{
    AC_RUN_ERROR << "System Call 'execve' not implemented yet." << std::endl;
}

AC_SYSCALL::fork()
{
    AC_RUN_ERROR << "System Call 'fork' not implemented yet." << std::endl;
}

AC_SYSCALL::wait()
{
    AC_RUN_ERROR << "System Call 'wait' not implemented yet." << std::endl;
}

#include <ac_syscall_codes.h>

AC_SYSCALL::ac_syscall_wrapper()
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

  case __NR_unlink:
    DEBUG_SYSCALL("unlink");
    ret = buf_stat.st_nlink;
    break;

  case __NR_stat:
    {
    DEBUG_SYSCALL("stat");
    get_buffer(0, pathname, 100);
    const char* path = (char*) pathname;
    ret = ::stat(path, &buf_stat);
    }
    break;

    /* Special cases for the fields of the "struct stat":
       to convert from glibc to newlib */

#define FILL_STRUCT_STAT(x)                             \
    case __AC_struct_stat_##x:                          \
      DEBUG_SYSCALL("filling struct stat field: " #x);  \
      ret = buf_stat.x;                                 \
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
    
  set_int(0, ret);
  return_from_syscall();
}

AC_SYSCALL::ac_syscall_geterrno()
{
  set_int(0, errno);
  return_from_syscall();
}

AC_SYSCALL::ac_syscall_stat_mode()
{
#if 0 /// Changed to iostream-type. --Marilia
    AC_RUN_ERROR("System Call ac_syscall_stat_mode not implemented yet.\n");
#else
    AC_RUN_ERROR << "System Call ac_syscall_stat_mode not implemented yet." << std::endl;
#endif
  exit(EXIT_FAILURE);
}

AC_SYSCALL::ac_forbidden()
{
  AC_ERROR("segmentation fault - PC at invalid address.");
  exit(EXIT_FAILURE);
}

AC_SYSCALL::ac_rtld_dummy()
{
  set_int(0, 0);
  return_from_syscall();
}

AC_SYSCALL::ac_rtld_init()
{  
#ifndef AC_COMPSIM
  unsigned ac_init_vecsz = ref.ac_dyn_loader.get_init_arraysz();
  unsigned *ac_init_vec = ref.ac_dyn_loader.get_init_array();
  if (ac_init_vecsz > 0) {
    unsigned destination = ac_init_vec[0];
    for (int i =0; i < ac_init_vecsz-1; i++)
      ac_init_vec[i] = ac_init_vec[i+1];
    ac_init_vecsz--;
    ref.ac_dyn_loader.set_init_arraysz(ac_init_vecsz);
    set_pc(destination);
    if (ac_init_vecsz == 0) {
      set_int(0, 0x84);
    } else {
      set_int(0, 0);
    }
    set_int(1, 0);
    set_int(2, 0);
    set_return(0x80);
  } else {
    AC_RUN_ERROR << "runtime dynamic loader: no init function.";
    exit(EXIT_FAILURE);
  }
#else
    AC_RUN_ERROR << "runtime dynamic loader: no init function.";
    exit(EXIT_FAILURE);
#endif
}

AC_SYSCALL::ac_rtld_fini()
{
#ifndef AC_COMPSIM
  static unsigned returnaddr = 0;
  unsigned ac_fini_vecsz = ref.ac_dyn_loader.get_fini_arraysz();
  unsigned *ac_fini_vec = ref.ac_dyn_loader.get_fini_array();

  if (returnaddr == 0) {
    returnaddr = get_return();
  }
    
  if (ac_fini_vecsz > 0) {
    unsigned destination = ac_fini_vec[0];
    for (int i =0; i < ac_fini_vecsz-1; i++)
      ac_fini_vec[i] = ac_fini_vec[i+1];
    ac_fini_vecsz--;
    ref.ac_dyn_loader.set_fini_arraysz(ac_fini_vecsz);
    set_pc(destination);
    set_return(0x84);
  } else {
    set_pc(returnaddr);
  }
#else
    AC_RUN_ERROR << "Error Syscalls: AC_RTLD Not implemented.";
    exit(EXIT_FAILURE);
#endif
}

#ifndef AC_COMPSIM

#include <sys/utsname.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>

#define SET_BUFFER_CORRECT_ENDIAN(addr, buf, size)                       \
  do {                                                                  \
    unsigned char *ptr = (unsigned char*) buf;                          \
    for (int ndx = 0; ndx < (size); ndx += sizeof(ac_word)) {           \
      *((ac_word *)(ptr + ndx)) = (ac_word)                             \
        convert_endian(sizeof(ac_word), (unsigned) *((ac_word *)(ptr + ndx)), \
                       ref.ac_mt_endian);                               \
    }                                                                   \
    host2guestmemcpy(addr, ptr, (size));                                \
  } while(0)

#define CORRECT_ENDIAN(word, size) (convert_endian((size),              \
                                                   (word), ref.ac_mt_endian))

#define CORRECT_STAT_STRUCT(buf)                                        \
  do{                                                                   \
    buf.st_dev     = CORRECT_ENDIAN(buf.st_dev, sizeof(dev_t));         \
    buf.st_ino     = CORRECT_ENDIAN(buf.st_ino, sizeof(ino_t));         \
    buf.st_mode    = CORRECT_ENDIAN(buf.st_mode, sizeof(mode_t));       \
    buf.st_nlink   = CORRECT_ENDIAN(buf.st_nlink, sizeof(nlink_t));     \
    buf.st_uid     = CORRECT_ENDIAN(buf.st_uid, sizeof(uid_t));         \
    buf.st_gid     = CORRECT_ENDIAN(buf.st_gid, sizeof(gid_t));         \
    buf.st_rdev    = CORRECT_ENDIAN(buf.st_rdev, sizeof(dev_t));        \
    buf.st_size    = CORRECT_ENDIAN(buf.st_size, sizeof(off_t));        \
    buf.st_blksize = CORRECT_ENDIAN(buf.st_blksize, sizeof(blksize_t)); \
    buf.st_blocks  = CORRECT_ENDIAN(buf.st_blocks, sizeof(blkcnt_t));   \
    buf.st_atime   = CORRECT_ENDIAN(buf.st_atime, sizeof(time_t));      \
    buf.st_mtime   = CORRECT_ENDIAN(buf.st_mtime, sizeof(time_t));      \
    buf.st_ctime   = CORRECT_ENDIAN(buf.st_ctime, sizeof(time_t));      \
  } while(0)

#define CORRECT_SOCKADDR_STRUCT_TO_HOST(buf)                            \
  do{                                                                   \
    buf.sa_family  = CORRECT_ENDIAN(buf.sa_family, sizeof(sa_family_t));\
    if (buf.sa_family == AF_INET) {                                     \
      struct sockaddr_in* tmp_sa_inet = (struct sockaddr_in*)&buf;      \
      tmp_sa_inet->sin_addr.s_addr = CORRECT_ENDIAN(tmp_sa_inet         \
                                                    ->sin_addr.s_addr,  \
                                                    sizeof(unsigned));  \
    } else if (buf.sa_family == AF_UNIX) {                              \
    } else if (buf.sa_family == AF_INET6) {                             \
      struct sockaddr_in6* tmp_sa_inet = (struct sockaddr_in6*)&buf;    \
      tmp_sa_inet->sin6_flowinfo = CORRECT_ENDIAN(tmp_sa_inet           \
                                                  ->sin6_flowinfo,      \
                                                  sizeof(unsigned));    \
      tmp_sa_inet->sin6_scope_id = CORRECT_ENDIAN(tmp_sa_inet           \
                                                  ->sin6_scope_id,      \
                                                  sizeof(unsigned));    \
    }                                                                   \
  } while(0)
#define CORRECT_SOCKADDR_STRUCT_TO_GUEST(buf)                           \
  do{                                                                   \
    if (buf.sa_family == AF_INET) {                                     \
      struct sockaddr_in* tmp_sa_inet = (struct sockaddr_in*)&buf;      \
      tmp_sa_inet->sin_addr.s_addr = CORRECT_ENDIAN(tmp_sa_inet         \
                                                    ->sin_addr.s_addr,  \
                                                    sizeof(unsigned));  \
    } else if (buf.sa_family == AF_UNIX) {                              \
    } else if (buf.sa_family == AF_INET6) {                             \
      struct sockaddr_in6* tmp_sa_inet = (struct sockaddr_in6*)&buf;    \
      tmp_sa_inet->sin6_flowinfo = CORRECT_ENDIAN(tmp_sa_inet           \
                                                  ->sin6_flowinfo,      \
                                                  sizeof(unsigned));    \
      tmp_sa_inet->sin6_scope_id = CORRECT_ENDIAN(tmp_sa_inet           \
                                                  ->sin6_scope_id,      \
                                                  sizeof(unsigned));    \
    }                                                                   \
    buf.sa_family  = CORRECT_ENDIAN(buf.sa_family, sizeof(sa_family_t));\
  } while(0)

#define CORRECT_TIMEVAL_STRUCT(buf)                                     \
  do{                                                                   \
    buf.tv_sec     = CORRECT_ENDIAN(buf.tv_sec, sizeof(time_t));        \
    buf.tv_usec    = CORRECT_ENDIAN(buf.tv_usec, sizeof(suseconds_t));  \
  } while(0)

#define CORRECT_TIMEZONE_STRUCT(buf)                                    \
  do{                                                                   \
    buf.tz_minuteswest = CORRECT_ENDIAN(buf.tz_minuteswest,             \
                                        sizeof(int));                   \
    buf.tz_dsttime     = CORRECT_ENDIAN(buf.tz_dsttime, sizeof(int));   \
  } while(0)


/* This function should be called by the syscall instruction
   behavior (INT, SYSCALL, SWI, etc.) of the model. It is an alternative
   to linking with libac_sysc (which creates binaries that uses the
   functions above. */
/* As syscall codes varies with the target, this implementation loads
   an array (get_syscall_table, function that should be overridden by
   the model syscall class) that contains the specific syscall code
   at each position (first position is reserved to "__NR_restart_syscall",
   second to "__NR_exit", etc.) */
template <class ac_word, class ac_Hword>
int ac_syscall<ac_word, ac_Hword>::process_syscall(int syscall) {
  const int *sctbl = get_syscall_table();
  
  if (sctbl == NULL)
     return -1;

  if (syscall == sctbl[0]) {        // restart_syscall

  } else if (syscall == sctbl[1]) { // exit
    DEBUG_SYSCALL("exit");
    int ac_exit_status = get_int(0);
#ifdef USE_GDB
    if (ref.get_gdbstub()) (ref.get_gdbstub())->exit(ac_exit_status);
#endif /* USE_GDB */
    ref.stop(ac_exit_status);
    return 0;

  } else if (syscall == sctbl[2]) { // fork
    int ret = ::fork();
    set_int(0, ret);
    return 0;
  } else if (syscall == sctbl[3]) { // read
    DEBUG_SYSCALL("read");
/*#ifdef AC_MEM_HIERARCHY
    if (!flush_cache()) return;
#endif  */
    int fd = get_int(0);
    unsigned count = get_int(2);
    unsigned char *buf = (unsigned char*) malloc(count);
    int ret = ::read(fd, buf, count);
    uint32_t guest_addr = get_int(1);
    host2guestmemcpy(guest_addr, buf, ret);
    set_int(0, ret);
    free(buf);
    return 0;

  } else if (syscall == sctbl[4]) { // write
    DEBUG_SYSCALL("write");
/*#ifdef AC_MEM_HIERARCHY
    if (!flush_cache()) return;
#endif*/
    int fd = get_int(0);
    unsigned count = get_int(2);
    unsigned char *buf = (unsigned char*) malloc(count);
    get_buffer(1, buf, count);
    int ret = ::write(fd, buf, count);
    set_int(0, ret);
    free(buf);
    return 0;

  } else if (syscall == sctbl[5]) { // open
    DEBUG_SYSCALL("open");
/*#ifdef AC_MEM_HIERARCHY
    if (!flush_cache()) return;
#endif*/
    unsigned char pathname[100];
    get_buffer(0, pathname, 100);
    int flags = convert_open_flags(get_int(1));
    int mode = get_int(2);
    int ret = ::open((char*)pathname, flags, mode);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[6]) { // close
    DEBUG_SYSCALL("close");
    int fd = get_int(0);
    int ret;
    // Silently ignore attempts to close standard streams (newlib may try to do so when exiting)
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
      ret = 0;
    else
      ret = ::close(fd);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[7]) { // creat
    DEBUG_SYSCALL("creat");
    unsigned char pathname[100];
    get_buffer(0, pathname, 100);
    int mode = get_int(1);
    int ret = ::creat((char*)pathname, mode);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[8]) { // time
    DEBUG_SYSCALL("time");
    time_t param;
    time_t ret = ::time(&param);
    if (get_int(0) != 0 && ret != (time_t)-1) {
      uint32_t guest_addr = get_int(0);
      SET_BUFFER_CORRECT_ENDIAN(guest_addr, (unsigned char *)&param,
                                (unsigned)sizeof(time_t));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[9]) { // lseek
    DEBUG_SYSCALL("lseek");
    off_t offset = get_int(1);
    int whence = get_int(2);
    int fd = get_int(0);
    int ret;
    ret = ::lseek(fd, offset, whence);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[10]) { // getpid
    DEBUG_SYSCALL("getpid");
    pid_t ret = getpid();
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[11]) { // access
    DEBUG_SYSCALL("access");
    unsigned char pathname[100];
    get_buffer(0, pathname, 100);
    int mode = get_int(1);
    int ret = ::access((char*)pathname, mode);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[12]) { // kill
    DEBUG_SYSCALL("kill");
    set_int(0, 0); 
    return 0;

  } else if (syscall == sctbl[13]) { // dup
    DEBUG_SYSCALL("dup");
    int fd = get_int(0);
    int ret = dup(fd);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[14]) { // times
    DEBUG_SYSCALL("times");
    struct tms buf;
    clock_t ret = ::times(&buf);
    if (ret != (clock_t)-1) {
      uint32_t guest_addr = get_int(0);
      SET_BUFFER_CORRECT_ENDIAN(guest_addr, (unsigned char *)&buf,
                                (unsigned)sizeof(struct tms));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[15]) { // brk
    DEBUG_SYSCALL("brk");
    int ptr = get_int(0);
    set_int(0, ref.ac_dyn_loader.mem_map.brk((Elf32_Addr)ptr));
    return 0;

  } else if (syscall == sctbl[16]) { // mmap
    DEBUG_SYSCALL("mmap");
    // Supports only anonymous mappings
    int flags = get_int(3);
    Elf32_Addr addr = get_int(0);
    Elf32_Word size = get_int(1);
    if (!is_mmap_anonymous(flags)) { // Not anonymous
      set_int(0, -EINVAL);
      fprintf(stderr, "not anonymous - returned error\n");
    } else {
      set_int(0, ref.ac_dyn_loader.mem_map.mmap_anon(addr, size));
    }
    return 0;

  } else if (syscall == sctbl[17]) { // munmap
    DEBUG_SYSCALL("munmap");
    Elf32_Addr addr = get_int(0);
    Elf32_Word size = get_int(1);
    if (ref.ac_dyn_loader.mem_map.munmap(addr, size))
      set_int(0, 0);
    else
      set_int(0, -EINVAL);
    return 0;

  } else if (syscall == sctbl[18]) { // stat
    DEBUG_SYSCALL("stat");
    unsigned char pathname[256];
    get_buffer(0, pathname, 256);
    struct stat buf;
    int ret = ::stat((char *)pathname, &buf);
    if (ret >= 0) {
      CORRECT_STAT_STRUCT(buf);
      uint32_t guest_addr = get_int(1);
      host2guestmemcpy(guest_addr, (unsigned char *)&buf, sizeof(struct stat));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[19]) { // lstat
    DEBUG_SYSCALL("lstat");
    unsigned char pathname[256];
    get_buffer(0, pathname, 256);
    struct stat buf;
    int ret = ::lstat((char *)pathname, &buf);
    if (ret >= 0) {
      CORRECT_STAT_STRUCT(buf);
      uint32_t guest_addr = get_int(1);
      host2guestmemcpy(guest_addr, (unsigned char *)&buf, sizeof(struct stat));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[20]) { // fstat
    DEBUG_SYSCALL("fstat");
    int fd = get_int(0);
    struct stat buf;
    int ret = ::fstat(fd, &buf);
    if (ret >= 0) {
      CORRECT_STAT_STRUCT(buf);
      uint32_t guest_addr = get_int(1);
      host2guestmemcpy(guest_addr, (unsigned char *)&buf, sizeof(struct stat));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[21]) { // uname
    DEBUG_SYSCALL("uname");
    struct utsname *buf = (struct utsname*) malloc(sizeof(utsname));
    int ret = ::uname(buf);
    uint32_t guest_addr = get_int(0);
    host2guestmemcpy(guest_addr, (unsigned char *) buf, sizeof(utsname));
    free(buf);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[22]) { // _llseek
    DEBUG_SYSCALL("_llseek");
    unsigned fd = get_int(0);
    unsigned long offset_high = get_int(1);
    unsigned long offset_low = get_int(2);
    int ret;
    unsigned whence = get_int(4);
    if (offset_high == 0) {
      int64_t ret_off;
      // Assume 64-bit offsets for all systems
      ret_off = ::lseek(fd, offset_low, whence);
      if (ret_off >= 0) {
        uint32_t guest_addr = get_int(3);
        uint64_t ret_off_cvt =
            ref.ac_mt_endian ? ret_off : (ret_off >> 32) | (ret_off << 32);
        SET_BUFFER_CORRECT_ENDIAN(guest_addr, (unsigned char *)&ret_off_cvt, 8);
        ret = 0;
      } else {
	ret = -1;
      }
    } else {
      ret = -1;
    }
    set_int(0, ret);
    return 0; 

  } else if (syscall == sctbl[23]) { // readv
    DEBUG_SYSCALL("readv");
    int ret;
    int fd = get_int(0);
    int iovcnt = get_int(2);
    // Read input assuming guest is 32-bit linux (4 bytes for pointer size)
    // Check struct iovec for more info
    const uint32_t guest_iovec_sz = 8;
    uint32_t *input = (uint32_t *) malloc(guest_iovec_sz * iovcnt);
    get_buffer(1, (unsigned char *) input, guest_iovec_sz * iovcnt);
    struct iovec *buf = (struct iovec *) malloc(sizeof(struct iovec) * iovcnt);
    for (int i = 0; i < iovcnt; i++) {
      unsigned char *tmp = (unsigned char *) malloc(input[2 * i + 1]);
      buf[i].iov_base = (void *)tmp;
      buf[i].iov_len = CORRECT_ENDIAN(input[2 * i + 1], 4);
    }
    ret = ::readv(fd, buf, iovcnt);
    for (int i = 0; i < iovcnt; i++) {
      host2guestmemcpy(CORRECT_ENDIAN(input[2 * i], 4),
                       (unsigned char *)buf[i].iov_base, buf[i].iov_len);
      free(buf[i].iov_base);
    }
    free(buf);
    free(input);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[24]) { // writev
    DEBUG_SYSCALL("writev");
    int ret;
    int fd = get_int(0);
    int iovcnt = get_int(2);
    struct iovec *buf = (struct iovec *) malloc(sizeof(struct iovec)*iovcnt);
    // Read input assuming guest is 32-bit linux (4 bytes for pointer size)
    const uint32_t guest_iovec_sz = 8;
    uint32_t *input = (uint32_t *) malloc(guest_iovec_sz * iovcnt);
    get_buffer(1, (unsigned char *) input, guest_iovec_sz * iovcnt);
    for (int i = 0; i < iovcnt; i++) {
      unsigned char *tmp;
      uint32_t endian_tmp = CORRECT_ENDIAN(input[2 * i], 4);
      buf[i].iov_len = CORRECT_ENDIAN(input[2 * i + 1], 4);
      tmp = (unsigned char *)malloc(buf[i].iov_len);
      buf[i].iov_base = (void *)tmp;
      guest2hostmemcpy(tmp, endian_tmp, buf[i].iov_len);
    }
    ret = ::writev(fd, buf, iovcnt);
    for (int i = 0; i < iovcnt; i++) {
      free(buf[i].iov_base);
    }
    free(buf);
    free(input);
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[25]) { // mmap2
    return process_syscall(sctbl[16]); //redirect to mmap

  } else if (syscall == sctbl[26]) { // stat64
    DEBUG_SYSCALL("stat64");
    unsigned char pathname[256];
    get_buffer(0, pathname, 256);
    struct stat64 buf;
    int ret = ::stat64((char *)pathname, &buf);
    if (ret >= 0) {
      CORRECT_STAT_STRUCT(buf);
      uint32_t guest_addr = get_int(1);
      host2guestmemcpy(guest_addr, (unsigned char *)&buf,
                       sizeof(struct stat64));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[27]) { // lstat64
    DEBUG_SYSCALL("lstat64");
    unsigned char pathname[256];
    get_buffer(0, pathname, 256);
    struct stat64 buf;
    int ret = ::lstat64((char *)pathname, &buf);
    if (ret >= 0) {
      CORRECT_STAT_STRUCT(buf);
      uint32_t guest_addr = get_int(1);
      host2guestmemcpy(guest_addr, (unsigned char *)&buf,
                       sizeof(struct stat64));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[28]) { // fstat64
    DEBUG_SYSCALL("fstat64");
    int fd = get_int(0);
    struct stat64 buf;
    int ret = ::fstat64(fd, &buf);
    if (ret >= 0) {
      CORRECT_STAT_STRUCT(buf);
      uint32_t guest_addr = get_int(1);
      host2guestmemcpy(guest_addr, (unsigned char *)&buf,
                       sizeof(struct stat64));
    }
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[29]) { // getuid32
    DEBUG_SYSCALL("getuid32");
    uid_t ret = ::getuid();
    set_int(0, (int)ret);
    return 0;

  } else if (syscall == sctbl[30]) { // getgid32
    DEBUG_SYSCALL("getgid32");
    gid_t ret = ::getgid();
    set_int(0, (int)ret);
    return 0;

  } else if (syscall == sctbl[31]) { // geteuid32
    DEBUG_SYSCALL("geteuid32");
    uid_t ret = ::geteuid();
    set_int(0, (int)ret);
    return 0;

  } else if (syscall == sctbl[32]) { // getegid32
    DEBUG_SYSCALL("getegid32");
    gid_t ret = ::getegid();
    set_int(0, (int)ret);
    return 0;

  } else if (syscall == sctbl[33]) { // fcntl64
    DEBUG_SYSCALL("fcntl64");
    int ret = -EINVAL;
    set_int(0, ret);
    return 0;

  } else if (syscall == sctbl[34]) { // exit_group
    DEBUG_SYSCALL("exit_group");
    int ac_exit_status = get_int(0);
#ifdef USE_GDB
    if (ref.get_gdbstub()) (ref.get_gdbstub())->exit(ac_exit_status);
#endif /* USE_GDB */
    ref.stop(ac_exit_status);
    return 0;
  } else if (syscall == sctbl[35]) { // socketcall
    DEBUG_SYSCALL("socketcall");
    // See target toolchain include/linux/net.h and include/asm/unistd.h
    // for detailed information on socketcall translation. This works
    // form ARM.
    int ret;
    int call = get_int(0);
    unsigned char tmp[256];
    get_buffer(1, tmp, 256);
    unsigned *args = (unsigned*) tmp;
    switch (call) {
    case 1: // Assuming 1 = SYS_SOCKET
      {
        DEBUG_SYSCALL("\tsocket");
        ret = ::socket(args[0], args[1], args[2]);
        break;
      }
    case 2: // Assuming 2 = SYS_BIND
      {
        DEBUG_SYSCALL("\tbind");
        struct sockaddr buf;
        set_int(0, args[1]);
        get_buffer(0, (unsigned char*)&buf, sizeof(struct sockaddr));
        CORRECT_SOCKADDR_STRUCT_TO_HOST(buf);
        ret = ::bind(args[0], &buf, args[2]);
        break;
      }
    case 3: // Assuming 3 = SYS_CONNECT
      {
        DEBUG_SYSCALL("\tconnect");
        struct sockaddr buf;
        set_int(0, args[1]);
        get_buffer(0, (unsigned char*)&buf, sizeof(struct sockaddr));
        CORRECT_SOCKADDR_STRUCT_TO_HOST(buf);
        ret = ::connect(args[0], &buf, args[2]);
        break;
      }
    case 4: // Assuming 4 = SYS_LISTEN
      {
        DEBUG_SYSCALL("\tlisten");
        ret = ::listen(args[0], args[1]);
        break;
      }
    case 5: // Assuming 5 = SYS_ACCEPT
      {
        struct sockaddr addr;
        socklen_t addrlen;
        DEBUG_SYSCALL("\taccept");
        ret = ::accept(args[0], &addr, &addrlen);
        CORRECT_SOCKADDR_STRUCT_TO_GUEST(addr);
        addrlen = CORRECT_ENDIAN(addrlen, sizeof(socklen_t));
        host2guestmemcpy(args[1], (unsigned char *)&addr,
                         sizeof(struct sockaddr));
        host2guestmemcpy(args[2], (unsigned char *)&addrlen, sizeof(socklen_t));
        break;
      }
    default:
      AC_WARN("Unimplemented socketcall() call number #" << call);
      break;
    }
    set_int(0, ret);
    return 0;
  } else if (syscall == sctbl[36]) { // gettimeofday
    DEBUG_SYSCALL("gettimeofday");
    int ret = -EINVAL;
    struct timezone tz;
    struct timeval tv;
    ret = ::gettimeofday(&tv, &tz);
    CORRECT_TIMEVAL_STRUCT(tv);
    CORRECT_TIMEZONE_STRUCT(tz);
    uint32_t guest_addr1 = get_int(0);
    host2guestmemcpy(guest_addr1, (unsigned char *)&tv, sizeof(struct timeval));
    uint32_t guest_addr2 = get_int(1);
    host2guestmemcpy(guest_addr2, (unsigned char *)&tz,
                     sizeof(struct timezone));
    set_int(0, ret);
    return 0;
  } else if (syscall == sctbl[37]) { // settimeofday
    DEBUG_SYSCALL("settimeofday");
    int ret = -EPERM;
    AC_WARN("settimeofday: Ignored attempt to change host date");
    set_int(0, ret);
    return 0;
  } else if (syscall == sctbl[38]) { // clock_gettime
    DEBUG_SYSCALL("clock_gettime");
    uint32_t clockid = get_int(0);
    uint32_t guest_addr = get_int(1);
    // Assume a 32-bit guest
    uint32_t guest_ts[2];
    #ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    guest_ts[0] = mts.tv_sec;
    guest_ts[1] = mts.tv_nsec;
    set_int(0, 0);
    #else
    struct timespec ts;
    int32_t ret = ::clock_gettime(clockid, &ts);
    guest_ts[0] = ts.tv_sec;
    guest_ts[1] = ts.tv_nsec;
    set_int(0, ret);
    #endif
    host2guestmemcpy(guest_addr, (unsigned char *)&guest_ts, 8);
    return 0;
  }

  /* Default case */
  set_int(0, -EINVAL);
  return -1;
}
#undef SET_BUFFER_CORRECT_ENDIAN
#undef CORRECT_ENDIAN
#undef CORRECT_STAT_STRUCT
#undef CORRECT_SOCKADDR_STRUCT_TO_HOST
#undef CORRECT_SOCKADDR_STRUCT_TO_GUEST
#undef CORRECT_TIMEVAL_STRUCT
#undef CORRECT_TIMEZONE_STRUCT

#endif // ifndef AC_COMPSIM

#endif

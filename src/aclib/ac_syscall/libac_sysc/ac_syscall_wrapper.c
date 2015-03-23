/**
 * @file      ac_syscall_wrapper.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
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

#include <ac_syscall_wrapper.h>
#include <ac_syscall_codes.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef ARM
#define unlink  _unlink
#define getpid  _getpid
#endif


_syscall2(int,link,const char *,oldpath,const char *,newpath)
_syscall1(int,unlink,const char*,pathname)

_syscall2(int,chmod,const char *,path,mode_t,mode);
_syscall2(int,fchmod,int,fildes,mode_t,mode);

_syscall3(int,chown,const char *,path,uid_t,owner,gid_t,group);
_syscall3(int,fchown,int,fd,uid_t,owner,gid_t,group);
_syscall3(int,lchown,const char *,path,uid_t,owner,gid_t,group);

_syscall0(pid_t,getpid);
_syscall0(pid_t,getppid);

_syscall1(int,dup,int,oldfd)
_syscall2(int,dup2,int,oldfd,int,newfd)


/* Specially handled syscalls */

/* _syscall2(int,stat,const char *,file_name,struct stat *,buf); */
/* _syscall2(int,fstat,int, filedes,struct stat *,buf); */
/* _syscall2(int,lstat,const char *,file_name,struct stat *,buf); */

#include <unistd.h>

#ifdef ARM
int _fstat(int filedes, struct stat *buf) {
    return (fstat(filedes,buf));
}
#endif

int fstat(int filedes, struct stat *buf)
{
  int res = ac_syscall_wrapper(__NR_fstat,filedes,buf);
  if (res == -1) {
    errno = ac_syscall_geterrno();
  }
  else {
    buf->st_dev = ac_syscall_wrapper(__AC_struct_stat_st_dev);    
    buf->st_ino = ac_syscall_wrapper(__AC_struct_stat_st_ino);    
    buf->st_mode = ac_syscall_wrapper(__AC_struct_stat_st_mode);   
    buf->st_nlink = ac_syscall_wrapper(__AC_struct_stat_st_nlink);  
    buf->st_uid = ac_syscall_wrapper(__AC_struct_stat_st_uid);    
    buf->st_gid = ac_syscall_wrapper(__AC_struct_stat_st_gid);    
    buf->st_rdev = ac_syscall_wrapper(__AC_struct_stat_st_rdev);   
    buf->st_size = ac_syscall_wrapper(__AC_struct_stat_st_size);   
    buf->st_blksize = ac_syscall_wrapper(__AC_struct_stat_st_blksize);
    buf->st_blocks = ac_syscall_wrapper(__AC_struct_stat_st_blocks); 
    buf->st_atime = ac_syscall_wrapper(__AC_struct_stat_st_atime);  
    buf->st_mtime = ac_syscall_wrapper(__AC_struct_stat_st_mtime);  
    buf->st_ctime = ac_syscall_wrapper(__AC_struct_stat_st_ctime);  
  }
  return res;
}

#include <utime.h>

int utime (const char *filename, const struct utimbuf *times)
{
  return 0;
}

void abort (void)
{
  _exit(1);
}

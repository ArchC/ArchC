/**
 * @file      stat.c
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
 */

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *path, struct stat *buf)
{
  int mode = ac_syscall_stat_mode(path);
  if (!mode) {
    errno = ac_syscall_geterrno();
    return -1;
  }
  else {
    //correct values:
    buf->st_mode = mode;
    //dummy values:
    buf->st_size = 1;
    buf->st_uid = 1;
    buf->st_gid = 1;
    return 0;
  }
}

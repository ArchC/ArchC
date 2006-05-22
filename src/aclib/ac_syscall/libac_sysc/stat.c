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

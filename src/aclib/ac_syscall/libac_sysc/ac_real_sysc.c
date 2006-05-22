/* some syscalls that trap to simulator functions */

#define AC_SYSC(NAME,LOCATION) \
int NAME() { \
  goto *LOCATION; \
}

#include "ac_syscall.def"

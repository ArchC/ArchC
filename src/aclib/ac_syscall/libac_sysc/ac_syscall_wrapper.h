/**
 * @file      ac_syscall_wrapper.h
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

#ifndef _AC_SYSCALL_WRAPPER_H_
#define _AC_SYSCALL_WRAPPER_H_

#include <errno.h>



#define __syscall_return(type, res) \
do { \
  if ((int)(res) == -1) errno = ac_syscall_geterrno(); \
  return (type) (res); \
} while (0)


#define _syscall0(type,name) \
type name(void) \
{ \
long __res; \
__res = ac_syscall_wrapper(__NR_##name); \
__syscall_return(type,__res); \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
long __res; \
__res = ac_syscall_wrapper(__NR_##name,arg1); \
__syscall_return(type,__res); \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1, type2 arg2) \
{ \
long __res; \
__res = ac_syscall_wrapper(__NR_##name,arg1,arg2); \
__syscall_return(type,__res); \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1, type2 arg2, type3 arg3) \
{ \
long __res; \
__res = ac_syscall_wrapper(__NR_##name,arg1,arg2,arg3); \
__syscall_return(type,__res); \
}

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
long __res; \
__res = ac_syscall_wrapper(__NR_##name,arg1,arg2,arg3,arg4); \
__syscall_return(type,__res); \
}

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) \
{ \
long __res; \
__res = ac_syscall_wrapper(__NR_##name,arg1,arg2,arg3,arg4,arg5); \
__syscall_return(type,__res); \
}

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
{ \
long __res; \
__res = ac_syscall_wrapper(__NR_##name,arg1,arg2,arg3,arg4,arg5,arg6); \
__syscall_return(type,__res); \
}


#endif /* _AC_SYSCALL_WRAPPER_H_ */

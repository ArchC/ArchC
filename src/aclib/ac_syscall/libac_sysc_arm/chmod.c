/**
 * @file      chmod.c
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
 * @note      chmod and fchmod not implemented yet
 */

#include <sys/types.h>
#include <sys/stat.h>

int chmod(const char *path, mode_t mode)
{
  return 0;
}

int fchmod(int fildes, mode_t mode)
{
  return 0;
}

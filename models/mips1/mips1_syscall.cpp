/********************************************************/
/* Acpp.c: The ArchC MIPS-I functional model.           */
/* Author: Sandro Rigo and Marcus Bartholomeu           */
/*                                                      */
/* For more information on ArchC, please visit:         */
/* http://www.archc.org                                 */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/
#include "mips1_syscall.H"

void mips1_syscall::get_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB.read(4+argn);

  for (unsigned int i = 0; i<size; i++, addr++) {
    buf[i] = DM.read_byte(addr);
  }
}

void mips1_syscall::set_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB.read(4+argn);

  for (unsigned int i = 0; i<size; i++, addr++) {
    DM.write_byte(addr, buf[i]);
  }
}

void mips1_syscall::set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB.read(4+argn);

  for (unsigned int i = 0; i<size; i+=4, addr+=4) {
    DM.write(addr, *(unsigned int *) &buf[i]);
  }
}

int mips1_syscall::get_int(int argn)
{
  return RB.read(4+argn);
}

void mips1_syscall::set_int(int argn, int val)
{
  RB.write(2+argn, val);
}

void mips1_syscall::return_from_syscall()
{
  ac_pc = RB.read(31);
}

void mips1_syscall::set_prog_args(int argc, char **argv)
{
  int i, j, base;

  unsigned int ac_argv[30];
  char ac_argstr[512];

  base = mips1_parms::AC_RAM_END - 512;
  for (i=0, j=0; i<argc; i++) {
    int len = strlen(argv[i]) + 1;
    ac_argv[i] = base + j;
    memcpy(&ac_argstr[j], argv[i], len);
    j += len;
  }

  //Ajust %sp and write argument string
  RB.write(29, mips1_parms::AC_RAM_END-512);
  set_buffer(25, (unsigned char*) ac_argstr, 512);   //$25 = $29(sp) - 4 (set_buffer adds 4)

  //Ajust %sp and write string pointers
  RB.write(29, mips1_parms::AC_RAM_END-512-120);
  set_buffer_noinvert(25, (unsigned char*) ac_argv, 120);

  //Set %o0 to the argument count
  RB.write(4, argc);

  //Set %o1 to the string pointers
  RB.write(5, mips1_parms::AC_RAM_END-512-120);
}

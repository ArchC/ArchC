#include <elf.h>
#include <libelf.h>
#include <elfutils/libdw.h>
#include <elfutils/libdwfl.h>
#include <dwarf.h>

extern char *appfilename;
void generate_trace_for_address(unsigned long long int addr);


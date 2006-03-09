

#ifndef _UTILS_H_
#define _UTILS_H_

#include "acpp.h"

/* Indentation */
#define IND1 "  "
#define IND2 "    "
#define IND3 "      "
#define IND4 "        "
#define IND5 "          "

extern unsigned int get_arch_size();
extern unsigned int get_insn_size(ac_asm_insn *insn);
extern void set_arch_name(char *str);
extern char *get_arch_name();
extern void internal_error();

extern int log_table[];

/* got from architecture file by the parser */
extern char *isa_filename;  
extern int ac_tgt_endian; /* filled by the parser 0=l, 1=b */

extern ac_dec_format *format_ins_list;


#endif /* _UTILS_H_ */

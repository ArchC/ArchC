

#ifndef _ARCHC_H_FILE_
#define _ARCHC_H_FILE_

#include <stdio.h>

/* This enum must be synced with acpp */
typedef enum {op_userdef, op_exp, op_imm, op_addr} operand_type;

/* Pointer to modifier encoder functions */
typedef void (*mod_enc_fnptr)(unsigned int, unsigned int, int, unsigned int *);


/* operand_modifier enum typedef */ 
___modifier_enum___


/* operand structure typedef */
typedef struct {
  const char *name;  /* operand type name */
  operand_type type;
  operand_modifier mod_type;
  unsigned int mod_addend;
  unsigned int fields;
  unsigned int format_id;
  unsigned int reloc_id;
} acasm_operand;



extern unsigned int operand_buffer[___max_fields___];
extern const mod_enc_fnptr modencfn[];
extern const unsigned int num_modencfn;
extern const acasm_operand operands[];
extern const unsigned int num_oper_id;




extern void print_operand_info(FILE *stream, unsigned int indent, unsigned int opid);


extern long long getbits(unsigned int bitsize, char *location, int endian); 
extern void putbits(unsigned int bitsize, char *location, long long value, int endian);

extern unsigned int get_num_fields(unsigned int encoded_field);
extern unsigned int get_field_id(unsigned int encoded_field, unsigned int pos);

extern void encode_cons_field(unsigned int *image, unsigned long val_const, unsigned int address, unsigned int oper_id);


extern unsigned long get_insn_size(unsigned long insn_fmt);
extern unsigned long ac_encode_insn(unsigned long insn_type, int field_id, unsigned long value, unsigned int *image);
extern unsigned int ac_get_field_value(unsigned int insn_type, int field_id, unsigned int value);
extern unsigned long get_field_size(unsigned long insn_fmt, int field_id);



/* modifiers prototypes */
___modifier_prototypes___








#endif /* _ARCHC_H_FILE_ */

#ifndef _OPC_H_FILE_
#define _OPC_H_FILE_

typedef struct {
  const char *mnemonic;
  const char *args;
  unsigned long image;
  unsigned long format_id;
  unsigned long pseudo_idx;
  unsigned long counter;
  unsigned long dmask;
} acasm_opcode;

typedef struct {
  const char *symbol;
  const char *cspec;
  unsigned long value;
} acasm_symbol;

extern long long getbits(unsigned int bitsize, char *location, int endian); 
extern void putbits(unsigned int bitsize, char *location, long long value, int endian);

extern const int num_opcodes;
extern acasm_opcode opcodes[];
extern const acasm_symbol udsymbols[];
extern const int num_symbols;
extern const char *pseudo_instrs[];
extern const int num_pseudo_instrs;

#endif

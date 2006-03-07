/* ex: set tabstop=2 expandtab: */

#ifndef `_TC_'___arch_name___`_H_FILE_'
#define `_TC_'___arch_name___`_H_FILE_'

#define ___endian_str___
#define AC_WORD_SIZE ___word_size___
#define TARGET_BYTES_BIG_ENDIAN ___endian_val___
#define TARGET_ARCH `bfd_arch_'___arch_name___
#define TARGET_FORMAT `"elf32-'___arch_name___`"'
#define WORKING_DOT_WORD
#define LOCAL_LABELS_FB 1

typedef struct fix_addend {
  unsigned long insn_format_id;
  unsigned long insn_field_id;
  int al_value;
  int hl_value;
  int flag;
  int field_size;
  int pcrel_add;
} archc_fix_addend;

#define TC_FIX_TYPE archc_fix_addend
#define TC_INIT_FIX_DATA(fixP)

extern int ___arch_name___`_parse_name'(char *name, expressionS *expP, char *c);
#define md_parse_name(x, y, z) ___arch_name___`_parse_name(x, y, z)'

extern void ___arch_name___`_frob_label'(symbolS *sym);
#define tc_frob_label(x) ___arch_name___`_frob_label'(x)

extern void ___arch_name___`_handle_align'(struct frag *);
#define HANDLE_ALIGN(fragp) ___arch_name___`_handle_align'(fragp)

extern void ___arch_name___`_cons_fix_new'(struct frag *, int, unsigned int, struct expressionS *);
#define TC_CONS_FIX_NEW ___arch_name___`_cons_fix_new'

#define TC_VALIDATE_FIX(fixp, this_segment, skip_label) \
  do \
    if (!___arch_name___`_validate_fix'((fixp), (this_segment))) \
      goto skip_label; \
  while (0)
extern int ___arch_name___`_validate_fix'(struct fix *, asection *);

#define DIFF_EXPR_OK            /* foo-. gets turned into PC relative relocs */
#endif

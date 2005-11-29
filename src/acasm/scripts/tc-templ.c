
#include "as.h"
#include "config.h"
#include "subsegs.h"
#include "safe-ctype.h"
#include "tc-xxxxx.h"
#include "opcode/xxxxx.h"

/*
  I will do it in ANSI C style =p (sorry K&R)
*/

/*
 Conversion specifiers masks and bit selection
 These are returned by get_builtin_marker()
*/
#define BI_NONE           0
#define BI_BASE_EXP_BIT   1
#define BI_BASE_ADDR_BIT (1 << 1)
#define BI_BASE_IMM_BIT  (1 << 2)
#define BI_MD_REL_BIT    (1 << 3)
#define BI_MD_HI_BIT     (1 << 4)
#define BI_MD_LO_BIT     (1 << 5)
#define BI_MD_AL_BIT     (1 << 6)
#define BI_MD_INV_BIT    (1 << 10)
#define BI_BASE_MASK (BI_BASE_EXP_BIT | BI_BASE_ADDR_BIT | BI_BASE_IMM_BIT)
#define BI_MD_MASK   (BI_MD_REL_BIT | BI_MD_HI_BIT | BI_MD_LO_BIT | BI_MD_AL_BIT)

/* FLAGS used with flag field in fixups */
#define FIX_FLAG_LO      0        /* low field */
#define FIX_FLAG_HI      1        /* high field */
#define FIX_FLAG_HIS    (1 << 1)  /* high field, signed */
#define FIX_FLAG_PCREL  (1 << 2)  /* pc relative */

#ifdef __STDC__
#define INTERNAL_ERROR() as_fatal (_("Internal error: line %d, %s"), __LINE__, __FILE__)
#else
#define INTERNAL_ERROR() as_fatal (_("Internal error"));
#endif



/* this comment is used by sed - don't ever change it */
static void s_cons(int byte_size);
static void archc_show_information(void);
static unsigned int get_builtin_marker(char *s);
static void ac_put_bits(unsigned int data, void *p, int bits);
static unsigned int ac_get_bits(const void *p, int bits);
static void pseudo_assemble(void);
static void emit_insn(void);
static int get_expression(expressionS *, char **);
static int get_operand(expressionS *ep, char **str);
static int ac_valid_symbol(char *ac_symbol, char *parser_symbol, 
                           long int *value);
static void ac_parse_operands(char *s_pos, char *args);
static void apply_hi_operator(unsigned long *value, 
                              unsigned long field_size);
static void sign_extend(unsigned long *value, int bit_size, 
                        int sign_bit);
static void get_addend(char addend_type, char **st);
static void create_fixup(unsigned int bi_type, long int field_id);
static void encode_cons_field(unsigned long val_const, unsigned int bi_type, long int field_id);


/*
  variables used by the core
*/
const char comment_chars[] = "#!@";  // # - MIPS, ! - SPARC, @ - ARM
const char line_comment_chars[] = "#";
const char line_separator_chars[] = ";";

const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "rRsSfFdDxXpP";
const pseudo_typeS md_pseudo_table[] = 
{ 
  {"half",  s_cons, AC_WORD_SIZE/8 /2},
  {"dword", s_cons, AC_WORD_SIZE/8 *2},
  {"byte",  s_cons, 1},
  {"hword", s_cons, AC_WORD_SIZE/8 /2},
  {"int",   s_cons, AC_WORD_SIZE/8},
  {"long",  s_cons, AC_WORD_SIZE/8},
  {"octa",  s_cons, AC_WORD_SIZE/8 *4},
  {"quad",  s_cons, AC_WORD_SIZE/8 *2},  
  {"short", s_cons, AC_WORD_SIZE/8 /2},
  {"word",  s_cons, AC_WORD_SIZE/8},
  {NULL, 0, 0},
};


/* hash tables: symbol and opcode */
static struct hash_control *sym_hash = NULL;
static struct hash_control *op_hash = NULL;

/* command line options */
static int ignore_undef = 0;
static int no_extern_rels = 0;
static int no_rels = 0;
static int start_text_addr = 0;
static int start_data_addr = 0;
static int data_after_text = 0;

/* assuming max input = 64 */
static int log_table[] = {  0 /*invalid*/,  0, 1, 1, 
                            2 /* log 4 */,  2, 2, 2, 
                            3 /* log 8 */,  3, 3, 3,
                            3,              3, 3, 3,
                            4 /* log 16 */, 4, 4, 4};

/* 
   Addend variables - set only by get_addend()
   Don't ever change these variables elsewhere!
*/
static int valhl = 0;  // value used to return H or L addends
static int valr = 0;   // value used to return R addends
static int vala = 0;   // value used to return A addends
static int hi_signed = 0; // if 1, high must be encoded signed


/* every error msg due to a parser error is stored here 
   NULL implies no error has occured  */
static char *insn_error;

/* a flag variable - if set, the parser is in 'imm' mode, that is, getting an
   operand of type imm */
static int in_imm_mode = 0;;

/* if an operand is of type expr, it will be saved here */
static expressionS ref_expr;

/* out_insn is the current instruction being assembled */
static struct insn_encoding
{
  unsigned long image;
  unsigned long format;

  unsigned int is_pseudo;
  unsigned int op_num;
  unsigned int op_pos[9*2];

  int need_fix;  
  int fix_pcrel_add; /* PC + x */
  int fix_apply_shift;
  int fix_apply_mask;
  int fix_flag;      /* see FIX_FLAG_*  */
  int fix_field_size;
  int fix_field_id;  /* field to be fixed */
} out_insn;




/*
  code
*/


void 
md_begin()
{
  int i;
  const char *retval;

  op_hash = hash_new();
  for (i = 0; i < num_opcodes;) {
    const char *t_mnemonic = opcodes[i].mnemonic;

    retval = hash_insert(op_hash, t_mnemonic, (PTR) &opcodes[i]);

    if (retval != NULL) {
      fprintf (stderr, _("internal error: can't hash '%s': %s\n"),
	       opcodes[i].mnemonic, retval);
      as_fatal (_("Broken assembler.  No assembly attempted."));
    }

    while ((++i < num_opcodes) && !strcmp(t_mnemonic, opcodes[i].mnemonic));

  }

  sym_hash = hash_new();
  for (i=0; i < num_symbols; i++) {
    const char *t_symbol = udsymbols[i].symbol;

    retval = hash_insert(sym_hash, t_symbol, (PTR) &udsymbols[i]);

    if (retval != NULL) {
      fprintf (stderr, _("internal error: can't hash `%s': %s\n"),
	       udsymbols[i].symbol, retval);
      as_fatal (_("Broken assembler.  No assembly attempted."));
    }
  }

  record_alignment(text_section, log_table[AC_WORD_SIZE/8]);

}



void 
md_assemble(char *str)
{
  static int has_pseudo = 0;

  /* insn we need to encode at this call */
  acasm_opcode *insn = NULL;

  /* we won't change str, so it always point to the start of the insn string.
     's_pos' will be used to parse it instead and points to the position in 
     'str' we are parsing at the moment */
  char *s_pos = str;

  /* a char used to store the value of the char in a string whenever we need 
     to break the original 'str' into substrings adding '\0', so we can
     restore the 'str' to full original string later */
  char save_c = 0;

  /* buffer used to store ArchC argument string and a pointer to scan it */
  char buffer[200];
  //  char *args = buffer;


  /* advance till first space (this indicates we should have got a mnemonic) 
     or end of string (maybe a mnemonic with no operands) */
  while (*s_pos != '\0' && !ISSPACE (*s_pos)) s_pos++;

  /* save the char at s_pos (it might get lost) */
  save_c = *s_pos;

  /* If we stopped on whitespace then replace the whitespace with null for
     the call to hash_find. Note that if we didn't stop on whitespace then
     *s_pos is already '\0': it doesn't hurt, does it? ;)  */
  *s_pos = '\0';

  /* try finding the opcode in the hash table */
  insn = (acasm_opcode *) hash_find (op_hash, str);

  /* if the instruction was not found then abort this insn encoding (it won't
     abort the assembler though, just move to the next one) */
  if (insn == NULL) {
    as_bad (_("unrecognized mnemonic: %s"), str);
    return;
  }

  /* restore original string */
  *s_pos = save_c;

  /* makes s_pos points to the start of the argument string (if there is one) */
  while (ISSPACE(*s_pos)) s_pos++;

  /* main encoding loop */
  for (;;) {
    strcpy(buffer, insn->args);

    out_insn.image = insn->image;
    out_insn.format = insn->format_id;
    out_insn.need_fix = 0;
    out_insn.fix_flag = FIX_FLAG_LO;  /* default */
    out_insn.fix_pcrel_add = 0;
    out_insn.op_num = 0;    
    out_insn.is_pseudo = insn->pseudo_idx;
    insn_error = NULL;
    ac_parse_operands(s_pos, buffer); /* parse the operands */

    /* try the next insn (overload) if the current failed */
    if (insn_error && (insn+1 < &opcodes[num_opcodes]) && 
        (!strcmp(insn->mnemonic, insn[1].mnemonic))) 
      insn++;    
    else break;
  } 

  if (insn_error) {
    as_bad(_("%s in insn: '%s'\n"), insn_error, str);
    return;
  }

  if (insn->pseudo_idx) {
    if (has_pseudo != 0) {
      as_bad(_("Pseudo-insn %s called by another pseudo-insn. Not assembled.'\n"), str);
      return;    
    }
    has_pseudo = 1;
    pseudo_assemble(); /* main code to handle pseudo instruction */
    has_pseudo = 0;
  }
  else 
    emit_insn();  /* save the instruction encoded in the the current frag */
}



/*
  Operands Parser. Actually, it does a semantic job as well.

  args -> points to the beginning of the argument list (from opcode table) 

  s_pos -> points to the beginning of the insn operand string

  The 'args' string comes from the opcode table generated automatically from an
  ArchC model and tells us how we should expect the syntax of the operands of
  an instruction.
*/  
static 
void ac_parse_operands(char *s_pos, char *args)
{
  /* an ArchC symbol value (gas symbols use expressionS) */
  long int symbol_value; 

  /* Archc marker for the current operand */
  //  char archc_marker;

  /* holds the field id within the format for the current operand */
  long int field_id;

  char save_c = 0;

  /* a buffer to hold the current conversion specifier found in args */
  char op_buf[30]; 
  char *ob = op_buf;

  insn_error = NULL;
  ref_expr.X_op = O_absent;

  /* while argument list is not empty, try parsing the operands */
  while (*args != '\0') { 

    /* a whitespace in the args string means that we need at least one
       whitespace in the s_pos string */
    if (ISSPACE(*args)){
      if (!ISSPACE(*s_pos)) {
        insn_error = "invalid instruction syntax";
        return;
      }
      args++;
      s_pos++;
      /* note that it's not possible to have a space as the last char of an
         'args' string, so we dont need to check for '\0' */
    }

    /* eats any space from s_pos so that multiples whitespaces can be handled correctly between
     syntatic elements - the gas pre-processor does this, but we must be sure xD */
    while (ISSPACE(*s_pos)) s_pos++; 

    /* if we've reached the end of insn operand string and not 'args', then
       some operands are missing like: "add" when we were expecting something
       like "add $03,$04,$05" */
    if (*s_pos == '\0') { 
      insn_error = "invalid instruction syntax";
      return;
    }

    /*
      '%' is the start of a well-formed sentence. It goes like this:

      '%'<conversion specifier>':'<insn field ID>':' 
    */
    if (*args == '%') {
      args++;

      /* save the start of the operand string in case a pseudo is called later */
      out_insn.op_pos[out_insn.op_num++] = (int)s_pos;

      /* ob points to the conversion specifier buffer - null terminated */
      ob = op_buf;
      while (*args != ':') { 
	*ob = *args;
	ob++; args++;
      }
      *ob = '\0';

      args++;                  /* start of the number field */

      /* parse the string to get the number */
      char *arg_start = args;
      while ((*args >= '0') && (*args <= '9')) args++;
      if (*args == *arg_start)  /* ops, no number */
	INTERNAL_ERROR();

      /* args is pointing to the character after the last number seen */
	
      /* if it's the end of args string then arg_start can be safely used to
	 extract the number. If it's not the last one we need to save the
	 actual char pointed by args and make it '\0', so arg_start can be
	 used as the argument to extract the number we are expecting.  

         Note:
	 Although it's useless to save args in case its a '\0' it's better than
	 making an 'if' branch.
      */
      save_c = *args;
      *args = '\0';
      field_id = atoi(arg_start);
      *args = save_c;

      if (*args != ':')  /* ops, bad argument syntax - check acasm ;) */
	INTERNAL_ERROR(); 
      
      args++;  /* points to the next valid syntatic element (after the ':') */

      unsigned int bi_type = get_builtin_marker(op_buf);

      /*
        main branch
        builtin specifier or user made
      */

      if (bi_type != BI_NONE) {
        /* builtin conversion specifier */

	if (bi_type & BI_MD_INV_BIT) /* ops, bad specifier - check acasm */
	  INTERNAL_ERROR();

	switch (bi_type & BI_BASE_MASK) {
	  
          /* Expression operand - 'expr' */
	case (BI_BASE_EXP_BIT):
          /* only suporting one symbol in an expression */

	  if (!get_expression(&ref_expr, &s_pos)) return;
	
	  if (ref_expr.X_op == O_symbol)    /* X_add_symbol + X_add_number */
            create_fixup(bi_type, field_id);

          else if (ref_expr.X_op == O_constant)  /* X_add_number */
            encode_cons_field(ref_expr.X_add_number, bi_type, field_id);
	  
	  else {
	    insn_error = "invalid expression";
	    return;
	  }
	  break;

          /* Immediate operand - 'imm' */
	case (BI_BASE_IMM_BIT):
	  in_imm_mode = 1;
	  if (!get_operand(&ref_expr, &s_pos)) {
	    in_imm_mode = 0;
	    return;
	  }
	  in_imm_mode = 0;
	  
          encode_cons_field(ref_expr.X_add_number, bi_type, field_id);

	  break;

          /* Address operand - 'addr' */
	case (BI_BASE_ADDR_BIT):
	  if (!get_expression(&ref_expr, &s_pos)) return;

	  if (ref_expr.X_op != O_symbol) {    // X_add_symbol + X_add_number
	    insn_error = "invalid operand, expected a symbolic address";
	    return;
	  }

          create_fixup(bi_type, field_id);	  
            
	  break;

	default:
	  INTERNAL_ERROR();

	}
      }
      else {
        /* user made conversion specifier */

	/* First decode the symbol to get its value */

	if (*args == '\0') { // args string has ended

	  /* TODO: May s have ' ' char at tail? If yes we should eliminate it before continuing. */

	  /*
	    Now we check if s points to a valid symbol matching the ArchC ac_symbol char.
	    In case of positive matching, arg_value will hold the symbol value as declared in the ArchC asm_symbol section.
	  */

	  if (!ac_valid_symbol(op_buf, s_pos, &symbol_value)) {
	    insn_error = "unrecognized operand symbol";
	    return;
	  }
	  	  
	  /* s_pos string has been recognized, so advance it till its end */
	  while (*s_pos != '\0') s_pos++;	  
	} 
	else { 
	  /*
	    We know args is not pointing to the end of string and so we expect to find more 
	    operands at the next loop interaction. So args is pointing to a char which s_pos should 
	    have after the operand we want to extract (usually its a char which separes one 
	    operand from another, like ','). After the operand we expect to extract from s_pos, 
	    s_pos should point to this same char args points to or to a ' ' char.
	  */
	  
	  /* save the start of the string where s_pos points to now */
	  char *s_start = s_pos;
	
	  /* now we advance s_pos till it reaches *args or end of string or ' ' */
	  while ((*s_pos != *args) && (*s_pos != ' ') && (*s_pos != '\0')) s_pos++;

	  if (*s_start == *s_pos) { /* add ,$3,$2 */
	    insn_error = "operand missing";
	    return;
	  }

	  if (*s_pos == '\0') { /* add $2 */
	    insn_error = "operand missing";
	    return;
	  }

	  /* we dont need to treat the case where s == ' ' because we know s was not pointing to a ' ' 
	     before and so we can safely say that s_start points to the start of a valid string (even 
	     if it is of size 1) */

	  save_c = *s_pos;
	  *s_pos = '\0';

	  if (!ac_valid_symbol(op_buf, s_start, &symbol_value)) {
	    //	    printf("-- %s --\n", op_buf);
	    insn_error = "unrecognized operand symbol";
	    *s_pos = save_c;
	    return;
	  }

	  *s_pos = save_c;	        	    
	}

	/* if we have reached here, we sucessfully got a valid symbol's value; so we encode it */
	out_insn.image |= ac_encode_insn(out_insn.format, field_id, symbol_value);

      } /* end of if's chain to decode operand action */

      out_insn.op_pos[out_insn.op_num++] = (int)s_pos;

    } /* end of if (*args == '%') */
    else {  
      /* args string must equal s_pos string in case its not a '%' */    

      /* scape */
      if (*args == '\\') {
	args++;		
      }
     
      if (*s_pos != *args) {	
	insn_error = "invalid instruction syntax";
	return;
      }
      args++;
      s_pos++;
      
    }
    
  } /* end while */

  if (*s_pos != '\0') {  /* have add $3,$2,$2[more] - expected add $3,$2,$2 */
    insn_error = "invalid instruction syntax";
    return;
  }
}



static void pseudo_assemble() {

  char op_string[9][50];
  unsigned int i;
  int j;

  /* extract the operand strings */
  for (i=0; i<out_insn.op_num/2; i++) {
     
    int str_count = out_insn.op_pos[i*2+1] - out_insn.op_pos[i*2];
    if (str_count < 0 || str_count >= 50) 
      INTERNAL_ERROR(); /* out buffer is too small */

    for (j=0; j<str_count; j++) 
      op_string[i][j] = *(char *)(out_insn.op_pos[i*2]+j);

    op_string[i][str_count] = '\0';

  }

  /* from now on the code must be reentrant (recursive calls to md_assemble) */
  int num_operands = (unsigned int) out_insn.op_num/2;
  int pseudo_ind = out_insn.is_pseudo;
  if (pseudo_ind > num_pseudo_instrs) INTERNAL_ERROR();

  char new_insn[50];

  while (pseudo_instrs[pseudo_ind] != NULL) {
    const char *pseudoP = pseudo_instrs[pseudo_ind];
    char *n_ind = new_insn;

    while (*pseudoP != '\0') {
      
      if (*pseudoP == '\\') 
	pseudoP++;
      else if (*pseudoP == '%') {
	pseudoP++;
	if ((*pseudoP < '0') || (*pseudoP > '9') || (*pseudoP-'0' >= num_operands)) 
	  INTERNAL_ERROR();

	strcpy(n_ind, op_string[*pseudoP-'0']);
	n_ind += strlen(op_string[*pseudoP-'0']);

	pseudoP++;
	continue;
      }

      *n_ind = *pseudoP;
      n_ind++;
      pseudoP++;

    }
    *n_ind = '\0';
      
    md_assemble(new_insn);

    pseudo_ind++;
  }
}


static void emit_insn() {

  /* frag address where we emit the instructions encoding (call frag_more() to get the address) */
  char *frag_address;

  /* Pretending we'll emit at least 32-bit */
  frag_address = frag_more(AC_WORD_SIZE/8);

  if (out_insn.need_fix) {
    fixS *fixP;

    if (ref_expr.X_add_symbol == NULL) 
      INTERNAL_ERROR();

    fixP = fix_new_exp (frag_now, frag_address - frag_now->fr_literal /* where */, AC_WORD_SIZE/8 /* size */,
			&ref_expr, (out_insn.fix_flag & FIX_FLAG_PCREL) ? 1 : 0, BFD_RELOC_NONE);

    if (!fixP) 
      INTERNAL_ERROR();
    
    fixP->tc_fix_data.insn_format_id = out_insn.format;
    fixP->tc_fix_data.insn_field_id = out_insn.fix_field_id;
    fixP->tc_fix_data.apply_mask = out_insn.fix_apply_mask;
    fixP->tc_fix_data.apply_shift = out_insn.fix_apply_shift; 
    fixP->tc_fix_data.flag = out_insn.fix_flag;
    fixP->tc_fix_data.field_size = out_insn.fix_field_size;
    fixP->tc_fix_data.pcrel_add = out_insn.fix_pcrel_add;

  }

  md_number_to_chars(frag_address, out_insn.image, AC_WORD_SIZE/8);

}



/*

  Function used to write 'val' in 'buf' using 'n' bytes with a correct machine endian.
  Usually it's called to emit instructions to a frag.

*/
void
md_number_to_chars(char *buf, valueT val, int n)
{
#ifdef AC_BIG_ENDIAN
    number_to_chars_bigendian (buf, val, n);  
#else
    number_to_chars_littleendian (buf, val, n);
#endif
}



/*

  Called by fixup_segment() (write.c) to apply a fixup to a frag.

  fixP -> a pointer to the fixup
  valP -> a pointer to the value to apply (with symbol value added)
  seg  -> segment which the fix is attached to

*/
void
md_apply_fix3 (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{

  /* We are going to check for undefined symbols here.
     This supposes an undefined symbol always is used as a reference from somewhere in code
     TODO: check for symbols defined but not used (labels)
   */
  if (!ignore_undef && fixP->fx_addsy != NULL)
    if (S_GET_SEGMENT(fixP->fx_addsy) == undefined_section) {
      if (fixP->fx_file)
	as_bad_where(fixP->fx_file, fixP->fx_line, _("undefined symbol %s"), 
		       S_GET_NAME(fixP->fx_addsy));
      else
	as_bad(_("undefined symbol %s used"), S_GET_NAME(fixP->fx_addsy));
    }

  // TODO: in case of validating undefined symbols, should we set a default value?

  //-----------------------------------------------

  /*
    If the relocation must be stored in reloc field (rather than in the insn itself) and the relocation
    is against a symbol (not a pc-relavite local)
    don't reloc anything against a symbol
   */
  if (no_rels && fixP->fx_addsy != NULL) {
    fixP->fx_done = 1;
    return;
  }
 

  /* relocate the contents by ourselves */
  bfd_vma x = 0;
  bfd_byte *location = (bfd_byte *) (fixP->fx_frag->fr_literal + fixP->fx_where);
  bfd_vma relocation = *valP; //fixP->fx_addnumber;


  /* the relocation is against a symbol... 
     if a start address has been specified, add the symbol's seg offset 
     note: no overflow checking done
  */

  if (fixP->fx_addsy != NULL) {
    if (S_GET_SEGMENT(fixP->fx_addsy) == data_section) {
      if (data_after_text)
        relocation += (bfd_section_size(stdoutput, text_section)/bfd_octets_per_byte(stdoutput))
          +start_text_addr;
      else
        relocation += start_data_addr;
    }
    else if (S_GET_SEGMENT(fixP->fx_addsy) == text_section)
      relocation += start_text_addr;
  }


  /* TODO: redo the get and put bit size logic
     Guess what happens if in a data section we find: '.byte label' 
     -> MIPS doesn't allow - SPARC gives internal error o.O
  */
  x = ac_get_bits(location, AC_WORD_SIZE);


  /* modifiers' applying order
     1 - PC relative (already applied by core)
     2 - high/low
     3 - alignment (and a sign extension)
  */


  if (fixP->tc_fix_data.flag & FIX_FLAG_HIS)
    apply_hi_operator(&relocation, fixP->tc_fix_data.field_size);
  

  int field_size = get_field_size(fixP->tc_fix_data.insn_format_id,
                                  fixP->tc_fix_data.insn_field_id);


  relocation &= fixP->tc_fix_data.apply_mask;

  if (fixP->tc_fix_data.apply_shift) { /* alignment and sign extension reajust */
    relocation >>= fixP->tc_fix_data.apply_shift; 
    sign_extend(&relocation, field_size, fixP->tc_fix_data.field_size-fixP->tc_fix_data.apply_shift);
  }


  if (fixP->tc_fix_data.insn_field_id != (unsigned int)-1) 
    x |= ac_encode_insn(fixP->tc_fix_data.insn_format_id, fixP->tc_fix_data.insn_field_id, 
			relocation);  
  else  /* simple relocations (data section) */
    x |= relocation;
     
  ac_put_bits(x, location, AC_WORD_SIZE);
 
  fixP->fx_done = TRUE; 
}


int
xxxxx_validate_fix(struct fix *fixP, asection *seg)
{

  /* force all fix-ups to be relocated */
  if (!no_extern_rels) return 1;


  /*
    don't adjust external fixups or the ones with undefined symbols, except the cases where an addend 
    must be encoded (like in MIPS: lw $2, %lo(extern+10)($10) --> 10 must be encoded anyway)
  */
  if ((S_IS_EXTERN(fixP->fx_addsy)) || (!S_IS_DEFINED (fixP->fx_addsy))) {
    if (fixP->fx_offset != 0)   /* offset = operand addend (constant) */
      md_apply_fix3(fixP, &fixP->fx_offset, seg);
    fixP->fx_done = 1;  /* don't call md_apply_fix3 anymore */
  }

  return 1;
}




/*

  Called by write_relocs() (write.c) for each fixup so that a BFD arelent structure can be build
and returned do be applied through 'bfd_install_relocation()' which in turn will call a backend
routine to apply the fix. As we are not dealing with relocations atm, we just return NULL so that
no bfd_install_relocation and similar functions will be called. 
*/
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixP ATTRIBUTE_UNUSED)
{
  // not generating BFD relocation entries right now
  return NULL;
}


void
xxxxx_handle_align (fragS *fragp  ATTRIBUTE_UNUSED)
{
/* We need to define this function so that in write.c, in routine 
 subsegs_finish, the variable alignment get the right size and the last frag 
 can be align.  
   I think there is another way to handle the alignment stuff without defining
this function (which is not mandatory). We just need to make md_section_align check 
the bfd alignment, and return the next aligned address. I've not tested that tho ;)
 */
}


/*
   Called (by operand()) when symbols might get their values taken like:
   .data
   .word label2
*/
extern void 
xxxxx_cons_fix_new(struct frag *frag, int where, unsigned int nbytes, struct expressionS *exp)
{
  fixS *fixP = NULL;

  /* todo: what if '.dword label' ? o.O
     same with '.byte label' -> we should flag it here and relocate it correctly in apply_fix3
  */

  fixP = fix_new_exp(frag, where, (int) nbytes, exp, 0 /*pcrel*/, BFD_RELOC_NONE);

  fixP->tc_fix_data.insn_format_id = (unsigned int) -1;  /* -1 is for data items */
  fixP->tc_fix_data.insn_field_id = (unsigned int) -1;
  fixP->tc_fix_data.apply_shift = 0;
  fixP->tc_fix_data.apply_mask = 0xFFFFFFFF >> (AC_WORD_SIZE - nbytes*8);
  fixP->tc_fix_data.flag = FIX_FLAG_LO;   // this must be expanded to FIX_FLAG_8, _16 (nbytes)
  fixP->tc_fix_data.field_size = nbytes*8;
  fixP->tc_fix_data.pcrel_add = 0;

  if (!fixP) INTERNAL_ERROR();

} 
 

/* ---------------------------------------------------------------------------------------
 Static functions
*/
 
/*
  We created this to auto-align data section whenever a data size > 1 is found
  (The MIPS version uses this method, so I decided to use also)
*/
static void
s_cons (int byte_size)
{
  if (byte_size > 1) {
    frag_align(log_table[byte_size], 0, 0);
    record_alignment(now_seg, log_table[byte_size]);
  }

  cons(byte_size);
}
 

static int
get_expression (expressionS *ep, char **str)
{
  char *save_in;

  save_in = input_line_pointer;

  input_line_pointer = *str;
  expression (ep);
  *str = input_line_pointer;

  input_line_pointer = save_in;
  
  if (insn_error) return 0;
  return 1;
}


static int
get_operand (expressionS *ep, char **str)
{
  char *save_in;

  save_in = input_line_pointer;

  input_line_pointer = *str;

  ep->X_op = O_constant;
  ep->X_add_number = get_single_number();

  *str = input_line_pointer;

  input_line_pointer = save_in;

  if (insn_error) return 0;
  return 1;
}


/* 
  if ac_symbol == "" then marker is not checked
*/
static int ac_valid_symbol(char *ac_symbol, char *parser_symbol, long int *value)
{
  acasm_symbol *msymb = NULL;
  
  msymb = (acasm_symbol *) hash_find (sym_hash, parser_symbol);

  // TODO: handle the case when there are 2 or more symbols with same names (but different markers)

  // symbol not found
  if (msymb == NULL) return 0;
  
  // marker not valid for this symbol
  if (ac_symbol != "")    
    if (strcmp(msymb->cspec, ac_symbol)) return 0;

  *value = msymb->value;

  return 1;
}




//-------------------------------------------------------------------------------------
// Necessary stuff
//-----------------------



/*
--------------------------------------------------------------------------------------
 Command-line parsing stuff


  'void parse_args(int *, char ***)' in <as.c> is the main routine called to parse command line options for the main GAS
 program. The machine dependent (md) part might extend the short and long options of the main GAS ones by means of these 
 variables: 

 (obrigatory variables)
    const char *md_shortopts  (short options)
    struct option md_longopts (long options)
    size_t md_longopts_size  (this must be set to the long options' size)

   Two other routines *MUST* be set:

 (obrigatory)
   int md_parse_option(int c, char *arg)
      'c' may be a character (in case of a short option) or a value in the 'val' field of a long option structure. If 'arg'
      is not NULL, then it holds a string related somehow to 'c' (an argument to 'c'). Strictly speaking, 'c' is the value
      returned by a call to the getopt_long_only(...) by parse_args.
      Return 0 to indicate the option 'c' was not recognized or !0 otherwise.

 (obrigatory)
   md_show_usage (FILE *stream)
      This is called by the main GAS show_usage routine and should display this assembler machine dependent options in 
      'stream'

   Still, there might be defined one more routine to do special parsing handling or md setting right after the options 
      parsing (its called at the end of 'parse_args' from <as.c>):

 (optional)
   void md_after_parse_args ()
--------------------------------------------------------------------------------------
*/
const char *md_shortopts = "";

struct option md_longopts[] =
{
  {"ignore-undef", no_argument, NULL, OPTION_MD_BASE},
  {"no-extern-rels", no_argument, NULL, OPTION_MD_BASE+1},  
  {"no-rels", no_argument, NULL, OPTION_MD_BASE+2},

  /* Note: these options don't generate a correct listing */
  {"text-addr", required_argument, NULL, OPTION_MD_BASE+3},
  {"data-addr", required_argument, NULL, OPTION_MD_BASE+4},

  {"archc", no_argument, NULL, OPTION_MD_BASE+5} 
};
size_t md_longopts_size = sizeof (md_longopts);


static void
archc_show_information()
{
  fprintf (stderr, _("GNU assembler automaticly generated by acasm beta version.\n"));
}


int
md_parse_option (int c, char *arg)
{
  switch (c) 
    {
    case OPTION_MD_BASE+0: /* ignore undefined symbols */
      ignore_undef = 1;
      break;

    case OPTION_MD_BASE+1:
      no_extern_rels = 1;
      break;

    case OPTION_MD_BASE+2:
      no_rels = 1;
      break;

    case OPTION_MD_BASE+3: /* .text addr */
      if (*arg == '0' && *(arg+1) == 'x') /* hexa */
        start_text_addr = strtol(arg, NULL, 16);
      else
        start_text_addr = atoi(arg);
      break;

    case OPTION_MD_BASE+4: /* .data addr */

      if (!strcmp(arg, ".text"))
        data_after_text = 1;
      else if (*arg == '0' && *(arg+1) == 'x') /* hexa */
        start_data_addr = strtol(arg, NULL, 16);
      else
        start_data_addr = atoi(arg);
      break;

    case OPTION_MD_BASE+5: /* display archc version information; */  
      archc_show_information();
      exit (EXIT_SUCCESS); 
      break;
     
    default:
      return 0;
  }

  return 1;
}


void
md_show_usage (FILE *stream)
{
  fprintf (stream, "md options:\n\n");

  fprintf (stream, _("\
  --ignore-undef          ignore undefined symbols\n"));
  fprintf (stream, _("\
  --no-extern-rels        do not perform relocation against extern symbols\n"));
  fprintf (stream, _("\
  --no-rels               do not perform internal relocation\n"));
  fprintf (stream, _("\
  --text-addr=<addr>      set the start address of .text section to <addr>\n"));
  fprintf (stream, _("\
  --data-addr=<addr>      set the start address of .data section to <addr>\n"));
  fprintf (stream, _("\
  --archc                 display ArchC information\n"));
}
/*---------------------------------------------------------------------------*/


char *
md_atof (type, litP, sizeP)
     int type ATTRIBUTE_UNUSED;
     char *litP ATTRIBUTE_UNUSED;
     int *sizeP ATTRIBUTE_UNUSED;
{
  return NULL;
}


/* Convert a machine dependent frag.  */
void
md_convert_frag (abfd, asec, fragp)
     bfd *abfd ATTRIBUTE_UNUSED;
     segT asec ATTRIBUTE_UNUSED;
     fragS *fragp ATTRIBUTE_UNUSED;
{
  return;
}


valueT
md_section_align (seg, addr)
     asection *seg ATTRIBUTE_UNUSED;
     valueT addr;
{
  //  int align = bfd_get_section_alignment (stdoutput, seg);

  /* We don't need to align ELF sections to the full alignment.
     However, Irix 5 may prefer that we align them at least to a 16
     byte boundary.  We don't bother to align the sections if we are
     targeted for an embedded system.  */
  //  if (strcmp (TARGET_OS, "elf") == 0)
    //    return addr;


  //  return ((addr + (1 << align) - 1) & (-1 << align));
  return addr;
}


int
md_estimate_size_before_relax (fragp, segtype)
     fragS *fragp ATTRIBUTE_UNUSED;
     asection *segtype ATTRIBUTE_UNUSED;
{
  return 0;
}


long
md_pcrel_from (fixP)
     fixS *fixP;
{
  return fixP->fx_where + fixP->fx_frag->fr_address + fixP->tc_fix_data.pcrel_add;
}

symbolS *md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

void md_operand (expressionS *expressionP)
{
  while (*input_line_pointer != '\0') input_line_pointer++;

  insn_error = "bad expression";
  expressionP->X_op = O_constant;
}


int xxxxx_parse_name(char *name, expressionS *expP, char *c ATTRIBUTE_UNUSED) {

  long int dummy;

  if (in_imm_mode) { /* no name allowed when getting an 'imm' operand */
    insn_error = "invalid operand, expected a number";
    return 1;
  }

  if (ac_valid_symbol("", name, &dummy)) { /* symbol found */
    expP->X_op = O_absent;
    insn_error = "invalid symbol in expression";
    return 1;
  }

  return 0;
}

void xxxxx_frob_label(symbolS *sym) {

  long int dummy;

  if (ac_valid_symbol("",  (char *)S_GET_NAME(sym), &dummy))  /* symbol found as label -.- */
    as_bad(_("invalid label name: '%s'"), S_GET_NAME(sym));
  
}


/*
  MIPS %hi operator adds the sign bit of the lower n-bits in the higher half
  so, we need to do %hi(X) = X - %lo(X) , where %lo(X) is a signed number
  check 'http://sources.redhat.com/ml/binutils/2004-08/msg00262.html' for further details
  SPARC doesn't use the %hi operator this way
*/
static 
void apply_hi_operator(unsigned long *value, unsigned long field_size) {

  int fsize = field_size;
  int lomask = 0xFFFFFFFF >> fsize;
  unsigned long low_number;
     
  low_number = *value & lomask;
  sign_extend(&low_number, 32, 32-field_size);

  *value -= low_number;      
}



/*
  value -> the value to be extended
  bit_size -> size (in bits) of the final field
  sign_bit -> position (from right to left) of the sign bit 1-based

  Example:
  value: 0xFFFB6
  bit_size: 22
  sign_bit: 20

  after: value = 0x3FFFB6

P.S. - considering 32-bit as the highest possible bit size
*/
static void
sign_extend(unsigned long *value, int bit_size, int sign_bit) 
{
  if (sign_bit == 0) return;
  sign_bit--;

  if ((1 << sign_bit) & *value) {
    *value |= ((0xFFFFFFFF >> (32-bit_size)) & (0xFFFFFFFF << sign_bit));
  }

}




/*
 Bases:
        'exp'
	'addr'
	'imm'

 Modifiers:
        'H'[s][n] s -> signed, n -> get the n-high bit
        'L'[n] -> get the n-low bits
	'R'[b][n] b -> backward, n -> add n to PC
	'A'[n] -> align in a paragraph of n-bits

 st : null-terminated string with the conversion specifier  
*/
static unsigned int 
get_builtin_marker(char *st)
{

  valhl = 0;
  valr  = 0;
  vala  = 0;
  hi_signed = 0;

  unsigned int ret_val = BI_NONE;

  if ( *st == 'e' && *(st+1) == 'x' && *(st+2) == 'p' ) {
    ret_val |= BI_BASE_EXP_BIT;
    st += 3;
  }
  else if ( *st == 'a' && *(st+1) == 'd' && *(st+2) == 'd' && *(st+3) == 'r' ) {
    ret_val |= BI_BASE_ADDR_BIT;
    st += 4;
  }
  else if ( *st == 'i' && *(st+1) == 'm' && *(st+2) == 'm') {
    ret_val |= BI_BASE_IMM_BIT;
    st += 3;
  }
  else return BI_NONE;

  while (*st != '\0') {
   
    switch (*st) {

    case 'H':
      ret_val |= BI_MD_HI_BIT;
      get_addend('H', &st);
      break;

    case 'L':
      ret_val |= BI_MD_LO_BIT;
      get_addend('L', &st);
      break;

    case 'A':
      ret_val |= BI_MD_AL_BIT;
      get_addend('A', &st);
      break;

    case 'R':
      ret_val |= BI_MD_REL_BIT;
      get_addend('R', &st);
      break;

    default:
      return ret_val | BI_MD_INV_BIT;
    }

    st++;
  }

  if (!(ret_val & BI_MD_HI_BIT))
    ret_val |= BI_MD_LO_BIT;

  return ret_val;
}



/*
  Parse and get the modifier's addend
  Only called by get_builtin_type()

  addend_type: 'H', 'L', 'A' or 'R' 

  st : string pointing to the addend type. It will be set to the next char
  after the addend
*/
static void 
get_addend(char addend_type, char **st) 
{
  char *sl = *st;
  sl++;
  if (addend_type == 'H' && *sl == 's') {
    hi_signed = 1;
    sl++;
    (*st)++;
  }
  else if (addend_type == 'R' && *sl == 'b') {
    *sl = '-';
    sl++;
    st++;
  }

  while (*sl >= '0' && *sl <= '9') sl++;
      
  if ((sl-1) != *st) {
    char savec = *sl;
    *sl = '\0';

    if (addend_type == 'H' || addend_type == 'L')
      valhl = atoi((*st)+1);
    else if (addend_type == 'A')
      vala = atoi((*st)+1); 
    else // 'R'
      valr = atoi((*st)+1);

    *sl = savec;
    *st = sl-1;
  }
}


/*
  Put and Get bits functions are based on the versions found in libbfd.c

  It's not the fastest way, but it's quite generic (8, 16, 24 and 32-bits)
*/
static void ac_put_bits(unsigned int data, void *p, int bits)
{
  bfd_byte *addr = p;
  int i;
  int bytes;

  if (bits % 8 != 0)
    INTERNAL_ERROR();

  bytes = bits / 8;
  for (i = 0; i < bytes; i++)
    {
#ifdef AC_BIG_ENDIAN
      int index = bytes - i - 1;
#else
      int index = i;
#endif

      addr[index] = data & 0xff;
      data >>= 8;
    }
}

static unsigned int ac_get_bits(const void *p, int bits)
{
  const bfd_byte *addr = p;
  bfd_uint64_t data;
  int i;
  int bytes;

  if (bits % 8 != 0)
    INTERNAL_ERROR();

  data = 0;
  bytes = bits / 8;
  for (i = 0; i < bytes; i++)
    {
#ifdef AC_BIG_ENDIAN
      int index = i;
#else
      int index = bytes - i - 1;
#endif

      data = (data << 8) | addr[index];
    }

  return data;
}



static void 
create_fixup(unsigned int bi_type, long int field_id) 
{
  int align =  (bi_type & BI_MD_AL_BIT) ? 1 : 0;
  int high = (bi_type & BI_MD_HI_BIT) ? 1 : 0;
  int pcrel = (bi_type & BI_MD_REL_BIT) ? 1 : 0;

  if (out_insn.need_fix)  /* only one fixup for instruction allowed */
    INTERNAL_ERROR();

  out_insn.need_fix = 1; 

  out_insn.fix_flag |= pcrel ? FIX_FLAG_PCREL : 0;
  out_insn.fix_pcrel_add = valr;
  out_insn.fix_apply_shift = align ? (vala ? log_table[vala] : log_table[AC_WORD_SIZE/8]) : 0;

  if (high) {
    out_insn.fix_flag |= hi_signed ? FIX_FLAG_HIS : FIX_FLAG_HI;
    out_insn.fix_field_size = valhl ? valhl : (int)get_field_size(out_insn.format, field_id);
    out_insn.fix_apply_shift += valhl ? AC_WORD_SIZE-valhl : AC_WORD_SIZE-(int)get_field_size(out_insn.format, field_id);
    out_insn.fix_apply_mask = 0xFFFFFFFF;
  }
  else { /* low */
    out_insn.fix_apply_mask = valhl ? 0xFFFFFFFF >> (32-valhl) : 0xFFFFFFFF >> (32-get_field_size(out_insn.format, field_id)); 
    out_insn.fix_field_size = valhl ? valhl : (int)get_field_size(out_insn.format, field_id);
  }
	    
  out_insn.fix_field_id = field_id;
}


static void 
encode_cons_field(unsigned long val_const, unsigned int bi_type, long int field_id) 
{
  int align =  (bi_type & BI_MD_AL_BIT) ? 1 : 0;
  int high = (bi_type & BI_MD_HI_BIT) ? 1 : 0;
  //  int pcrel = (bi_type & BI_MD_REL_BIT) ? 1 : 0;
	    
  /* DO NOT ADD PCREL IN EXPRESSIONS OPERANDS */
  if (high) {
    int high_value = valhl ? valhl : (int)get_field_size(out_insn.format, field_id);
    if (hi_signed)
      apply_hi_operator(&val_const, high_value);
    
    val_const >>= (AC_WORD_SIZE - high_value);
  }
  else 
    val_const &= valhl ? 0xFFFFFFFF >> (AC_WORD_SIZE-valhl) : 0xFFFFFFFF >> (AC_WORD_SIZE-get_field_size(out_insn.format, field_id));

  val_const >>= align ? (vala ? log_table[vala] : log_table[AC_WORD_SIZE/8]) : 0;
  out_insn.image |= ac_encode_insn(out_insn.format, field_id, val_const);
}

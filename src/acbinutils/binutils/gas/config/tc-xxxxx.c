/* ex: set tabstop=2 expandtab: */

#include "as.h"
#include "config.h"
#include "subsegs.h"
#include "safe-ctype.h"
#include `"tc-'___arch_name___`.h"'
#include `"elf/'___arch_name___`.h"'
#include `"opcode/'___arch_name___`.h"'


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
#define FIX_FLAG_LO        0        /* low field */
#define FIX_FLAG_HI        1        /* high field */
#define FIX_FLAG_HI_CARRY (1 << 1)  /* high field with carry from lower bits */
#define FIX_FLAG_PCREL    (1 << 2)  /* pc relative */
#define FIX_FLAG_ALIGNED  (1 << 3)  /* aligned (power of 2) */
#define FIX_FLAG_HL_SIGN  (1 << 4)
#define FIX_FLAG_AL_SIGN  (1 << 5)
#define FIX_FLAG_LO_DATA  (1 << 6)

#ifdef __STDC__
#define INTERNAL_ERROR() as_fatal (_("Internal error: line %d, %s"), __LINE__, __FILE__)
#else
#define INTERNAL_ERROR() as_fatal (_("Internal error"));
#endif


static unsigned long ac_encode_insn(unsigned long insn_type, int field_id, unsigned long value);
static unsigned long get_field_size(unsigned long insn_fmt, int field_id);
static unsigned long get_insn_size(unsigned long insn_fmt);
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
static int ac_parse_operands(char *s_pos, char *args);
static void apply_hi_operator(unsigned long *value, 
                              unsigned long field_size);
static void sign_extend(unsigned long *value, int bit_size, 
                        int sign_bit);
static void get_addend(char addend_type, char **st);
static void create_fixup(unsigned int bi_type, long int field_id, unsigned reloc_id);
static void encode_cons_field(unsigned long val_const, unsigned int bi_type, long int field_id);
static unsigned get_id(char **strP);
static void strtolower(char *str);

/*
  variables used by the core
*/
const char comment_chars[] = "#!";  // # - MIPS, ! - SPARC, @ - ARM
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


typedef unsigned char uc;
static const unsigned char charmap[] = {
	(uc)'\000',(uc)'\001',(uc)'\002',(uc)'\003',(uc)'\004',(uc)'\005',(uc)'\006',(uc)'\007',
	(uc)'\010',(uc)'\011',(uc)'\012',(uc)'\013',(uc)'\014',(uc)'\015',(uc)'\016',(uc)'\017',
	(uc)'\020',(uc)'\021',(uc)'\022',(uc)'\023',(uc)'\024',(uc)'\025',(uc)'\026',(uc)'\027',
	(uc)'\030',(uc)'\031',(uc)'\032',(uc)'\033',(uc)'\034',(uc)'\035',(uc)'\036',(uc)'\037',
	(uc)'\040',(uc)'\041',(uc)'\042',(uc)'\043',(uc)'\044',(uc)'\045',(uc)'\046',(uc)'\047',
	(uc)'\050',(uc)'\051',(uc)'\052',(uc)'\053',(uc)'\054',(uc)'\055',(uc)'\056',(uc)'\057',
	(uc)'\060',(uc)'\061',(uc)'\062',(uc)'\063',(uc)'\064',(uc)'\065',(uc)'\066',(uc)'\067',
	(uc)'\070',(uc)'\071',(uc)'\072',(uc)'\073',(uc)'\074',(uc)'\075',(uc)'\076',(uc)'\077',
	(uc)'\100',(uc)'\141',(uc)'\142',(uc)'\143',(uc)'\144',(uc)'\145',(uc)'\146',(uc)'\147',
	(uc)'\150',(uc)'\151',(uc)'\152',(uc)'\153',(uc)'\154',(uc)'\155',(uc)'\156',(uc)'\157',
	(uc)'\160',(uc)'\161',(uc)'\162',(uc)'\163',(uc)'\164',(uc)'\165',(uc)'\166',(uc)'\167',
	(uc)'\170',(uc)'\171',(uc)'\172',(uc)'\133',(uc)'\134',(uc)'\135',(uc)'\136',(uc)'\137',
	(uc)'\140',(uc)'\141',(uc)'\142',(uc)'\143',(uc)'\144',(uc)'\145',(uc)'\146',(uc)'\147',
	(uc)'\150',(uc)'\151',(uc)'\152',(uc)'\153',(uc)'\154',(uc)'\155',(uc)'\156',(uc)'\157',
	(uc)'\160',(uc)'\161',(uc)'\162',(uc)'\163',(uc)'\164',(uc)'\165',(uc)'\166',(uc)'\167',
	(uc)'\170',(uc)'\171',(uc)'\172',(uc)'\173',(uc)'\174',(uc)'\175',(uc)'\176',(uc)'\177',
	(uc)'\200',(uc)'\201',(uc)'\202',(uc)'\203',(uc)'\204',(uc)'\205',(uc)'\206',(uc)'\207',
	(uc)'\210',(uc)'\211',(uc)'\212',(uc)'\213',(uc)'\214',(uc)'\215',(uc)'\216',(uc)'\217',
	(uc)'\220',(uc)'\221',(uc)'\222',(uc)'\223',(uc)'\224',(uc)'\225',(uc)'\226',(uc)'\227',
	(uc)'\230',(uc)'\231',(uc)'\232',(uc)'\233',(uc)'\234',(uc)'\235',(uc)'\236',(uc)'\237',
	(uc)'\240',(uc)'\241',(uc)'\242',(uc)'\243',(uc)'\244',(uc)'\245',(uc)'\246',(uc)'\247',
	(uc)'\250',(uc)'\251',(uc)'\252',(uc)'\253',(uc)'\254',(uc)'\255',(uc)'\256',(uc)'\257',
	(uc)'\260',(uc)'\261',(uc)'\262',(uc)'\263',(uc)'\264',(uc)'\265',(uc)'\266',(uc)'\267',
	(uc)'\270',(uc)'\271',(uc)'\272',(uc)'\273',(uc)'\274',(uc)'\275',(uc)'\276',(uc)'\277',
	(uc)'\300',(uc)'\341',(uc)'\342',(uc)'\343',(uc)'\344',(uc)'\345',(uc)'\346',(uc)'\347',
	(uc)'\350',(uc)'\351',(uc)'\352',(uc)'\353',(uc)'\354',(uc)'\355',(uc)'\356',(uc)'\357',
	(uc)'\360',(uc)'\361',(uc)'\362',(uc)'\363',(uc)'\364',(uc)'\365',(uc)'\366',(uc)'\367',
	(uc)'\370',(uc)'\371',(uc)'\372',(uc)'\333',(uc)'\334',(uc)'\335',(uc)'\336',(uc)'\337',
	(uc)'\340',(uc)'\341',(uc)'\342',(uc)'\343',(uc)'\344',(uc)'\345',(uc)'\346',(uc)'\347',
	(uc)'\350',(uc)'\351',(uc)'\352',(uc)'\353',(uc)'\354',(uc)'\355',(uc)'\356',(uc)'\357',
	(uc)'\360',(uc)'\361',(uc)'\362',(uc)'\363',(uc)'\364',(uc)'\365',(uc)'\366',(uc)'\367',
	(uc)'\370',(uc)'\371',(uc)'\372',(uc)'\373',(uc)'\374',(uc)'\375',(uc)'\376',(uc)'\377',
};

static int case_insensitive = 0;

/* hash tables: symbol and opcode */
static struct hash_control *sym_hash = NULL;
static struct hash_control *op_hash = NULL;

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

static int is_hl_signed = 0;
static int is_alig_signed = 1; // align defaults to signed
static int high_has_carry = 0;

/* every error msg due to a parser error is stored here 
   NULL implies no error has occured  */
static char *insn_error;

/* a flag variable - if set, the parser is in 'imm' mode, that is, getting an
   operand of type imm */
static int in_imm_mode = 0;

/* if an operand is of type expr, it will be saved here */
static expressionS ref_expr;


typedef struct _acfixuptype
{
  expressionS ref_expr_fix; /* symbol in which the relocation points to */
  int fix_pcrel_add; /* PC + x */
  int fix_al_value;
  int fix_hl_value;
  int fix_flag;      /* see FIX_FLAG_*  */
  int fix_field_size;
  int fix_field_id;  /* field to be fixed */
  unsigned relocation_number;
  struct _acfixuptype *next;
} acfixuptype;


/* out_insn is the current instruction being assembled */
static struct insn_encoding
{
  unsigned long image;
  unsigned long format;

  unsigned int is_pseudo;
  unsigned int op_num;
  unsigned int op_pos[9*2];

  acfixuptype *fixup;
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

  /* TODO:
   *  . It not allowed to define the same symbol name in more than one
   *  ac_asm_map section -> remove this restriction
   *  . Desalloc the memory allocated in case of 'insensitive'
   */
  sym_hash = hash_new();
  for (i=0; i < num_symbols; i++) {
    const char *t_symbol = udsymbols[i].symbol;
    char *symtoadd = NULL;

    if (case_insensitive) {
      symtoadd  = (char *) malloc(strlen(t_symbol)+1);
      strcpy(symtoadd, t_symbol);
      strtolower(symtoadd);
    }
    else
      symtoadd = (char *)t_symbol;

    retval = hash_insert(sym_hash, (const char *)symtoadd, (PTR) &udsymbols[i]);

    if (retval != NULL) {
      fprintf (stderr, _("internal error: can't hash `%s': %s\n"),
         udsymbols[i].symbol, retval);
      as_fatal (_("Broken assembler.  No assembly attempted."));
    }
  }

  record_alignment(text_section, log_table[AC_WORD_SIZE/8]);

}


#define KEEP_GOING 1
#define STOP_INSN  0

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
   // TODO: Clean any allocated memory in fixup
    out_insn.fixup = NULL;
    out_insn.op_num = 0;    
    out_insn.is_pseudo = insn->pseudo_idx;
    insn_error = NULL;

    if (case_insensitive) {
      strtolower(s_pos);
      strtolower(buffer);
    }
    
    int go_next = ac_parse_operands(s_pos, buffer); /* parse the operands */

    if (go_next == STOP_INSN)
      break;
    
    /* try the next insn (overload) if the current failed */
    if (insn_error && (insn+1 < &opcodes[num_opcodes]) && 
        (!strcmp(insn->mnemonic, insn[1].mnemonic))) 
      insn++;    
    else break;
  } 

  if (insn_error) {
    as_bad(_("%s in insn: '%s'"), insn_error, str);
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
int ac_parse_operands(char *s_pos, char *args)
{
  /* an ArchC symbol value (gas symbols use expressionS) */
  long int symbol_value; 

  /* holds the first field id within the format for the current operand */
  long int field_id1;
  /* second id - TODO: make it generic? */
  long int field_id2;

  /* relocation id's - field 1 and field 2 - TODO: make it generic? */ 
  long int reloc_id_f1;
  long int reloc_id_f2;
 
  /* 1 if field 2 is needed for the same operand */
  int need_f2;
  
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
        return KEEP_GOING;
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
      return KEEP_GOING;
    }

    /*
      '%' is the start of a well-formed sentence. It goes like this:

      '%'<conversion specifier>':'<insn field ID>':<reloc_number>' 
    */
    if (*args == '%') {    
      args++;

      need_f2 = 0;

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

      field_id1 = get_id(&args);

      if (*args != ':')  /* ops, bad argument syntax - check acasm ;) */
        INTERNAL_ERROR(); 

      args++;  /* points to the next valid syntatic element (after the ':') */

       /* Parse the relocation number field */
      reloc_id_f1 = get_id(&args);


      if (*args == '+') { /* another field for the same operand is needed */
        args++;
        need_f2 = 1;
        field_id2 = get_id(&args);

        if (*args != ':')
          INTERNAL_ERROR();

        args++;

        reloc_id_f2 = get_id(&args);

        if (*args != ':')
          INTERNAL_ERROR();

      }
      else if (*args != ':')
        INTERNAL_ERROR();

      args++;

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

            if (!get_expression(&ref_expr, &s_pos)) 
              return KEEP_GOING;

            switch (ref_expr.X_op) {
              case O_symbol:  /* X_add_symbol + X_add_number */
                create_fixup(bi_type, field_id1, reloc_id_f1);
                if (need_f2) 
                  create_fixup(bi_type, field_id2, reloc_id_f2);
                break;
                
              case O_constant: /* X_add_number */
                if (need_f2) {
                  encode_cons_field(ref_expr.X_add_number >> get_field_size(out_insn.format, field_id2), bi_type, field_id1);
                  encode_cons_field(ref_expr.X_add_number, bi_type, field_id2);
                }
                else encode_cons_field(ref_expr.X_add_number, bi_type, field_id1);                
                break;

              case O_uminus:          /* (- X_add_symbol) + X_add_number.  */
              case O_bit_not:         /* (~ X_add_symbol) + X_add_number.  */
              case O_logical_not:     /* (! X_add_symbol) + X_add_number.  */
              case O_multiply:        /* (X_add_symbol * X_op_symbol) + X_add_number.  */
              case O_divide:          /* (X_add_symbol / X_op_symbol) + X_add_number.  */
              case O_modulus:         /* (X_add_symbol % X_op_symbol) + X_add_number.  */
              case O_left_shift:      /* (X_add_symbol << X_op_symbol) + X_add_number. */
              case O_right_shift:     /* (X_add_symbol >> X_op_symbol) + X_add_number. */
              case O_bit_inclusive_or:/* (X_add_symbol | X_op_symbol) + X_add_number.  */
              case O_bit_or_not:      /* (X_add_symbol |~ X_op_symbol) + X_add_number. */
              case O_bit_exclusive_or:/* (X_add_symbol ^ X_op_symbol) + X_add_number.  */
              case O_bit_and:         /* (X_add_symbol & X_op_symbol) + X_add_number.  */
                insn_error = "operation not supported with relocatable symbol";
                return STOP_INSN;
                break;

             default:
                insn_error = "invalid expression";
                return KEEP_GOING;
                
            }
          break;


          /* Immediate operand - 'imm' */
          case (BI_BASE_IMM_BIT):
            in_imm_mode = 1;
            if (!get_operand(&ref_expr, &s_pos)) {
              in_imm_mode = 0;
              return KEEP_GOING;
            }
            in_imm_mode = 0;

            if (need_f2) {
              encode_cons_field(ref_expr.X_add_number >> get_field_size(out_insn.format, field_id2), bi_type, field_id1);
              encode_cons_field(ref_expr.X_add_number, bi_type, field_id2);
            }
            else encode_cons_field(ref_expr.X_add_number, bi_type, field_id1);
          break;

          /* Address operand - 'addr' */
          case (BI_BASE_ADDR_BIT):
            if (!get_expression(&ref_expr, &s_pos)) return KEEP_GOING;

            if (ref_expr.X_op != O_symbol) {    // X_add_symbol + X_add_number
              insn_error = "invalid operand, expected a symbolic address";
              return KEEP_GOING;
            }

            create_fixup(bi_type, field_id1, reloc_id_f1);
            if (need_f2) create_fixup(bi_type, field_id2, reloc_id_f2);

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
            return KEEP_GOING;
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
            return KEEP_GOING;
          }

          if (*s_pos == '\0') { /* add $2 */
            insn_error = "operand missing";
            return KEEP_GOING;
          }

             /* we dont need to treat the case where s == ' ' because we know s was not pointing to a ' ' 
                before and so we can safely say that s_start points to the start of a valid string (even 
                if it is of size 1) */

          save_c = *s_pos;
          *s_pos = '\0';

          if (!ac_valid_symbol(op_buf, s_start, &symbol_value)) {
                //      printf("-- %s --\n", op_buf);
            insn_error = "unrecognized operand symbol";
            *s_pos = save_c;
            return KEEP_GOING;
          }

          *s_pos = save_c;                
        }

        /* NEVER!!! discomment the line below O.O */
        /* if (out_insn.is_pseudo) return; */

        /* if we have reached here, we sucessfully got a valid symbol's value; so we encode it */
        if (need_f2) {
          out_insn.image |= ac_encode_insn(out_insn.format, field_id1, symbol_value >> get_field_size(out_insn.format, field_id2));
          out_insn.image |= ac_encode_insn(out_insn.format, field_id2, symbol_value);
        }
        else
          out_insn.image |= ac_encode_insn(out_insn.format, field_id1, symbol_value);


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
        return KEEP_GOING;
      }
      args++;
      s_pos++;

    }
    
  } /* end while */

  if (*s_pos != '\0') {  /* have add $3,$2,$2[more] - expected add $3,$2,$2 */
    insn_error = "invalid instruction syntax";
    return KEEP_GOING;
  }

  return KEEP_GOING;
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
  frag_address = frag_more(get_insn_size(out_insn.format)/8);


  while (out_insn.fixup != NULL) {

    fixS *fixP;

    ref_expr = out_insn.fixup->ref_expr_fix;

    if (ref_expr.X_op != O_symbol)
      INTERNAL_ERROR();
    
    fixP = fix_new_exp (frag_now, frag_address - frag_now->fr_literal /* where */, get_insn_size(out_insn.format)/8 /* size */,
    &ref_expr, (out_insn.fixup->fix_flag & FIX_FLAG_PCREL) ? 1 : 0, out_insn.fixup->relocation_number ? out_insn.fixup->relocation_number : BFD_RELOC_NONE);
    
    if (!fixP)
      INTERNAL_ERROR();

    fixP->tc_fix_data.insn_format_id = out_insn.format;
    fixP->tc_fix_data.insn_field_id = out_insn.fixup->fix_field_id;
    fixP->tc_fix_data.al_value = out_insn.fixup->fix_al_value;
    fixP->tc_fix_data.hl_value = out_insn.fixup->fix_hl_value; 
    fixP->tc_fix_data.flag = out_insn.fixup->fix_flag;
    fixP->tc_fix_data.field_size = out_insn.fixup->fix_field_size;
    fixP->tc_fix_data.pcrel_add = out_insn.fixup->fix_pcrel_add;

    acfixuptype *p = out_insn.fixup;
    out_insn.fixup = out_insn.fixup->next;
    free(p); 
  }
  
#ifdef PRINT_FIXUP
///    printf("Symbol - %s :\n", S_GET_NAME(ref_expr.X_add_symbol));
///    printf("field size = %x\n", out_insn.fix_field_size);
///    printf("hl value = %x\n", out_insn.fix_hl_value);
///    printf("pcrel add = %x\n", out_insn.fix_pcrel_add);
///    printf("al value = %x\n", out_insn.fix_al_value);
///    printf("fix flag = %x\n", out_insn.fix_flag);
///    printf("field id = %x\n", out_insn.fix_field_id);
///    printf("reloc = %x\n\n", S_GET_VALUE(ref_expr.X_add_symbol));  
#endif


  md_number_to_chars(frag_address, out_insn.image, get_insn_size(out_insn.format)/8);

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
  // If it's a relative data relocation, change the type to the corresponding generic relative one
  if (fixP->fx_pcrel) {
    switch (fixP->fx_r_type) {
      case BFD_GENERIC_8:
        fixP->fx_r_type = BFD_GENERIC_REL8;
      break;
      case BFD_GENERIC_16:
        fixP->fx_r_type = BFD_GENERIC_REL16;
      break;
      case BFD_GENERIC_32:
        fixP->fx_r_type = BFD_GENERIC_REL32;
      break;
      default:
    /* If it still is a pcrel type (extern symbol), we add to the addend field
     * the value of the PC offset (architecture dependent feature)
     */ 
        fixP->fx_offset = -fixP->tc_fix_data.pcrel_add;  
      break;
    }
  }

  if (fixP->fx_r_type != BFD_RELOC_NONE && fixP->fx_addsy != NULL) // If there is a relocation, let the linker fix it
    return; 

  /* Apply local PC-relative relocations */
  /*
  *  Note: DO NOT test fx_r_type for BFD_RELOC_NONE, since it is possible that
  *   it is different from NONE, but even so it must be resolved locally
  *   ie, external PC-relative type, but with local symbol
  *   In the latter case, no relocation is generated since it was already solved here
  *
  *   TODO: this should call bfd_install_relocation()
  */
  if (fixP->fx_addsy == NULL) { /* PC-relative */
     
    bfd_vma x = 0;
    bfd_byte *location = (bfd_byte *) (fixP->fx_frag->fr_literal + fixP->fx_where);
    bfd_vma relocation = *valP; //fixP->fx_addnumber;
        
    x = ac_get_bits(location, get_insn_size(fixP->tc_fix_data.insn_format_id));
    if (fixP->tc_fix_data.flag & FIX_FLAG_ALIGNED) 
      relocation >>= fixP->tc_fix_data.al_value;
    
    x |= ac_encode_insn(fixP->tc_fix_data.insn_format_id, fixP->tc_fix_data.insn_field_id, 
          relocation);  
   
    ac_put_bits(x, location, get_insn_size(fixP->tc_fix_data.insn_format_id));
   
    fixP->fx_done = TRUE; 
  }  
  else { /* should not reach this area */
    INTERNAL_ERROR();  
  }  
}


int
___arch_name___`_validate_fix'(struct fix *fixP ATTRIBUTE_UNUSED, asection *seg ATTRIBUTE_UNUSED)
{
  return 1;
}




/*

  Called by write_relocs() (write.c) for each fixup so that a BFD arelent structure can be build
and returned do be applied through 'bfd_install_relocation()' which in turn will call a backend
routine to apply the fix. As we are not dealing with relocations atm, we just return NULL so that
no bfd_install_relocation and similar functions will be called. 
*/
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *rel;
  bfd_reloc_code_real_type r_type;

  rel = (arelent *) xmalloc (sizeof (arelent));
  rel->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  *rel->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  rel->address = fixp->fx_frag->fr_address + fixp->fx_where;

  r_type = fixp->fx_r_type;
  //rel->addend = fixp->fx_addnumber;
  rel->addend = fixp->fx_offset;
  rel->howto = bfd_reloc_type_lookup (stdoutput, r_type);

  if (rel->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
        _("Cannot represent relocation type %s"),
        bfd_get_reloc_code_name (r_type));
      /* Set howto to a garbage value so that we can keep going.  */
      rel->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_32);
      assert (rel->howto != NULL);
    }

  return rel;
}

char *
___arch_name___`_canonicalize_symbol_name'(char *c)
{
  if (case_insensitive) {
    strtolower(c);
  }

  return c;  
}

void
___arch_name___`_handle_align' (fragS *fragp  ATTRIBUTE_UNUSED)
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
   Called (by emit_expr()) when symbols might get their values taken like:
   .data
   .word label2
*/
extern void 
___arch_name___`_cons_fix_new'(struct frag *frag, int where, unsigned int nbytes, struct expressionS *exp)
{
  fixS *fixP = NULL;

  switch (nbytes) {
    case 1:
      fixP = fix_new_exp (frag, where, (int) nbytes, exp, 0, BFD_GENERIC_8);
      break;
    case 2:
      fixP = fix_new_exp (frag, where, (int) nbytes, exp, 0, BFD_GENERIC_16);
      break;
    case 4:
      fixP = fix_new_exp (frag, where, (int) nbytes, exp, 0, BFD_GENERIC_32);
      break;
    default:
      INTERNAL_ERROR();
  }

  fixP->tc_fix_data.insn_format_id = (unsigned int) -1;  /* -1 is for data items */
  fixP->tc_fix_data.insn_field_id = (unsigned int) -1;
  fixP->tc_fix_data.al_value = 0;
  fixP->tc_fix_data.hl_value = nbytes*8;
  fixP->tc_fix_data.flag = FIX_FLAG_LO_DATA;   
  fixP->tc_fix_data.field_size = AC_WORD_SIZE;//nbytes*8;
  //fixP->tc_fix_data.pcrel_add = 0;
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
const char *md_shortopts = "i";

struct option md_longopts[] =
{
  {"insensitive", no_argument, NULL, OPTION_MD_BASE},
  {"archc", no_argument, NULL, OPTION_MD_BASE+1} 
};
size_t md_longopts_size = sizeof (md_longopts);


static void
archc_show_information()
{
  fprintf (stderr, _("GNU assembler automatically generated by acbinutils 2.0beta1.\n\
Architecture name: ___arch_name___.\n"));
}


int
md_parse_option (int c, char *arg ATTRIBUTE_UNUSED)
{
  switch (c) 
  {
    case OPTION_MD_BASE+0:
    case 'i':
      case_insensitive = 1;
      break;
     
    case OPTION_MD_BASE+1: /* display archc version information; */  
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
  -i,--insensitive        use case-insensitive symbols\n"));
  
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
  if (fixP->fx_addsy == NULL)
    INTERNAL_ERROR();

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


int ___arch_name___`_parse_name'(char *name, expressionS *expP, char *c ATTRIBUTE_UNUSED) {

  long int dummy;

  if (in_imm_mode) { /* no name allowed when getting an 'imm' operand */
    insn_error = "invalid operand, expected a number";

    expP->X_op = O_absent;       /* some machines crash without these lines */
    expP->X_add_symbol = NULL; 
    return 1;
  }

  if (ac_valid_symbol("", name, &dummy)) { /* symbol found */
    expP->X_op = O_absent;
    insn_error = "invalid symbol in expression";
    return 1;
  }

  return 0;
}

void ___arch_name___`_frob_label'(symbolS *sym) {

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
  bit_size -> size (in bits) of the final field (returned value)
  sign_bit -> position (from right to left) of the sign bit (1-based)

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
        'H'[n][c][u|s] -> n -> get the n-high bit -- u or s (unsigned or unsigned) -- c - add carry from lower bits
        'L'[n][u|s] -> get the n-low bits
  'R'[n][b] -> backward, n -> add n to PC
  'A'[n][u|s] -> align in a paragraph of n-bits

 st : null-terminated string with the conversion specifier  
*/
static unsigned int 
get_builtin_marker(char *st)
{

  valhl = 0;
  valr  = 0;
  vala  = 0;
  is_hl_signed = 0;
  is_alig_signed = 1;
  high_has_carry = 0;

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
    case 'h': /* because of insensitive */
      ret_val |= BI_MD_HI_BIT;
      get_addend('H', &st);
      break;

    case 'L':
    case 'l': /* because of insensitive */
      ret_val |= BI_MD_LO_BIT;
      get_addend('L', &st);
      break;

    case 'A':
    case 'a': /* because of insensitive */
      ret_val |= BI_MD_AL_BIT;
      get_addend('A', &st);
      break;
    
  case 'r':
    case 'R':
      ret_val |= BI_MD_REL_BIT;
      get_addend('R', &st);
      break;

    default:
      INTERNAL_ERROR();
      //return ret_val | BI_MD_INV_BIT;
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

  /*
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
  */

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
    //    *st = sl-1;
  }

  switch (addend_type) {

  case 'H': // [c][u|s]
    if (*sl == 'c') {
      sl++;
      high_has_carry = 1;
    }
    // fall through
    /*
    if (*sl == 's') {
      sl++;
      is_hl_signed = 1;
    }
    else if (*sl == 'u') {
      sl++;
      is_hl_signed = 0;
    }
    break;
    */
  case 'L': // [u|s]
    if (*sl == 's') {
      sl++;
      is_hl_signed = 1;
    }
    else if (*sl == 'u') {
      sl++;
      is_hl_signed = 0;
    }
    break;
 
  case 'R': // [b]
    if (*sl == 'b') {
      sl++;
      valr = valr * (-1);
    }
    break;
 
  case 'A': // [u|s]
    if (*sl == 's') {
      sl++;
      is_alig_signed = 1;
    }
    else if (*sl == 'u') {
      sl++;
      is_alig_signed = 0;
    }
    break;
  }

  *st = sl-1;
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
create_fixup(unsigned int bi_type, long int field_id, unsigned reloc_id) 
{
  int align =  (bi_type & BI_MD_AL_BIT) ? 1 : 0;
  int high = (bi_type & BI_MD_HI_BIT) ? 1 : 0;
  int pcrel = (bi_type & BI_MD_REL_BIT) ? 1 : 0;

  if (out_insn.is_pseudo) return;

//  if (out_insn.need_fix)  /* only one fixup for instruction allowed */
//    INTERNAL_ERROR();
  acfixuptype *fixups;
  
  if (out_insn.fixup == NULL)
  {
    out_insn.fixup = (acfixuptype *) malloc(sizeof(acfixuptype));
   out_insn.fixup->next = NULL;  
   fixups = out_insn.fixup;
  }
  else {
    fixups = out_insn.fixup;
    while (fixups->next != NULL) fixups = fixups->next;
    fixups->next = (acfixuptype *) malloc(sizeof(acfixuptype));
    fixups = fixups->next;
    fixups->next = NULL;
  }

//  out_insn.need_fix = 1;   
  fixups->fix_flag = 0;

  fixups->fix_field_size = (int)get_field_size(out_insn.format, field_id);
  fixups->fix_hl_value = valhl ? valhl : (int)get_field_size(out_insn.format, field_id);
  fixups->fix_pcrel_add = valr;
  fixups->fix_al_value = align ? (vala ? log_table[vala] : log_table[AC_WORD_SIZE/8]) : 0;

  // PCREL fixup settings
  fixups->fix_flag |= pcrel ? FIX_FLAG_PCREL : 0;

  // ALIGN fixup settings
  fixups->fix_flag |= align ? FIX_FLAG_ALIGNED : 0;
  fixups->fix_flag |= align ? (is_alig_signed ? FIX_FLAG_AL_SIGN : 0) : 0;

  // HIGH/LOW fixup settings
  fixups->fix_flag |= high ? (high_has_carry ? FIX_FLAG_HI_CARRY : FIX_FLAG_HI) : FIX_FLAG_LO;
  fixups->fix_flag |= is_hl_signed ? FIX_FLAG_HL_SIGN : 0;

  fixups->fix_field_id = field_id;
  fixups->ref_expr_fix = ref_expr;

  fixups->relocation_number = reloc_id;
}


static void 
encode_cons_field(unsigned long val_const, unsigned int bi_type, long int field_id) 
{
  int align =  (bi_type & BI_MD_AL_BIT) ? 1 : 0;
  int high = (bi_type & BI_MD_HI_BIT) ? 1 : 0;
  //  int pcrel = (bi_type & BI_MD_REL_BIT) ? 1 : 0;
      
  if (out_insn.is_pseudo) return;

  /* DO NOT ADD PCREL IN EXPRESSIONS OPERANDS */
  if (high) {
    int high_value = valhl ? valhl : (int)get_field_size(out_insn.format, field_id);
    if (high_has_carry)
      apply_hi_operator(&val_const, high_value);
    
    val_const >>= (get_insn_size(out_insn.format) - high_value);
  }
  else 
    val_const &= valhl ? 0xFFFFFFFF >> (get_insn_size(out_insn.format)-valhl) : 0xFFFFFFFF >> (get_insn_size(out_insn.format)-get_field_size(out_insn.format, field_id));

  val_const >>= align ? (vala ? log_table[vala] : log_table[AC_WORD_SIZE/8]) : 0;
  out_insn.image |= ac_encode_insn(out_insn.format, field_id, val_const);
}


/*
 * strP -> pointer to the start of the buffer after ':'
 * at end -> strP : will point to the character after the number field
 */
static unsigned get_id(char **strP) {
  
  /* parse the string to get the number */
  char *arg_start = *strP;
  while ((**strP >= '0') && (**strP <= '9')) (*strP)++;
  if (**strP == *arg_start)  /* ops, no number */
    INTERNAL_ERROR();

  char save_c = **strP;
  **strP = '\0';
  unsigned field = atoi(arg_start);
  **strP = save_c;

  return field;
}


static void strtolower(char *str)
{
  while (*str != '\0') {
    *str = (uc)charmap[(uc)*str];
    str++;
  }
}



static unsigned long ac_encode_insn(unsigned long insn_type, int field_id, unsigned long value)
{
___encoding_function___
}

static unsigned long get_field_size(unsigned long insn_fmt, int field_id)
{
___fieldsize_function___
}

static unsigned long get_insn_size(unsigned long insn_fmt)
{
___insnsize_function___
}

/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/

/*  ArchC Language Grammar for GNU Bison
    Copyright (C) 2002-2004  The ArchC Team
    Please see license file distributed with this file.
    
    Visit http://www.archc.org for more information.
*/


/*
 * C declaration block
 */
%{
/*! \file archc_parser.c
 * \brief ArchC parser
 *
 * This file contains the generated bison grammar rules for
 * the ArchC language.
*/

/*! \defgroup bison_group Bison generated parser
 * \ingroup acpp_group
 *
 * The ArchC parser is generated using the GNU tools Bison and Flex.
 * The ArchC grammar for the parser and regular expressions for
 * the lexical analyser are stored into archc_grammar.y and
 * archc_lex.l files, respectively.
 *
 * @{
*/
#include <stdarg.h>
#include "ac_tools_common.h"
#include "core_actions.h"
#include "asm_actions.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE
#define ADD_DEBUG 1

/*!type used to identify which unit is being used for storage devices */
enum _sto_unit {BYTE = 1, KBYTE = 1024, MBYTE = 1048576, GBYTE = 1073741824};
typedef enum _sto_unit sto_unit;

/*!type used to identify the kind of list being declared */
enum _commalist {INSTR_L, STAGE_L, REG_L, PIPE_L, REGBANK_L, CACHE_L, MEM_L, TLM_PORT_L, TLM2_PORT_L, TLM2_NB_PORT_L, TLM_INTR_PORT_L, TLM2_INTR_PORT_L, INSTR_REF_L};
typedef enum _commalist commalist;

/*!type used to identify which description is being parsed */
enum _descr {ISA_D, ARCH_D};
typedef enum _descr description;

/* buffer variables */

static char* current_type;             //!< The name of the insn type/format associated to the object currently being parsed.
static ac_dec_instr* current_instr;    //!< Pointer to the current instruction
static ac_dec_format* current_format;  //!< Pointer to the current format
static ac_grp_list* current_group;     //!< Pointer to the current group
static sto_unit current_unit;          //!< Indicates what storage unit is being used.
static ac_pipe_list* current_pipe;     //!< Pointer to the current pipelinel;
static ac_pipe_list* anonymous_pipe = NULL;
static ac_sto_types cache_type;        //!< Indicates the type of cache being declared.

static commalist list_type;            //!< Indicates what type of list of declarations is being parsed.
static char error_msg[500];
static int is_in_old_setasm_mode;

static description descrp;             //!< Indicates what type of description is being parsed.

static int parse_error = 0;            //!< Indicates parse error occurred.

/* global */
char* project_name;                   //!< Name of the ArchC project being processed.
char* upper_project_name;             //!< Name of the ArchC project being processed, converted to uppercase.
char* isa_filename;                   //!< Name for the isa class file.
char* helper_contents;                //!< Contents of the ac_helper declaration.
int wordsize;                         //!< Size of the word type in bits for the current project.
int fetchsize;                        //!< Size of the fetch word type in bits for the current project.
int fetchbuffersize;                  //!< Size of the fetch buffer in bytes for the current project.
int ac_tgt_endian;                    //!< Indicate the endianess of the target architecture.
int force_setasm_syntax;              //!< tools should set this flag if the new set_asm syntax is required

/* lexer interface variable */
int line_num;                         //!< Input file line counter.

/*! Error reporting function
 * \param[in] format A string to be used as error message.
 */
static void yyerror(const char* format, ...)
{
 va_list args;
 va_start(args, format);
 parse_error++;
 fprintf(stderr, "ArchC ERROR: Line %d: ", line_num);
 vfprintf(stderr, format, args);
 fprintf (stderr, ".\n");
 va_end(args);
}

/*! Warning reporting function
 * \param[in] format A string to be used as warning message.
 */
static void yywarn(const char* format, ...)
{
 va_list args;
 va_start(args, format);
 fprintf(stderr, "ArchC WARNING: Line %d: ", line_num);
 vfprintf(stderr, format, args);
 fprintf(stderr, ".\n");
 va_end(args);
}

/*@}*/

%}  /* End of C declarations */



/*******************
 BISON Declarations
*********************/

/* General used Tokens */
%token <value> INT
%token <text> STR
%token <text> BLOCK
%token <text> DOT "."
%token <text> COMMA ","
%token <text> SEMICOLON ";"
%token <text> COLON ":"
%token <text> PLUS "+"
%token <text> LT "<"
%token <text> GT ">"
%token <text> EQ "="
%token <text> RPAREN ")"
%token <text> LPAREN "("
%token <text> LBRACK "["
%token <text> RBRACK "]"
%token <text> LBRACE "{"
%token <text> RBRACE "}"
%token <text> PP

/* ISA Tokens */
%token <text> SET_DECODER
%token <text> SET_ASM
%token <text> SET_CYCLES
%token <text> ID
%token <text> AC_ISA_UPPER
%token <text> ISA_CTOR
%token <text> AC_FORMAT
%token <text> AC_INSTR
%token <text> CYCLE_RANGE
%token <text> IS_JUMP IS_BRANCH COND DELAY DELAY_COND BEHAVIOR
%token <text> AC_ASM_MAP PSEUDO_INSTR
%token <text> AC_ASM_ASSEMBLER
%token <text> MAP_TO
%token <text> AC_ASM_COMMENT AC_ASM_LINE_COMMENT
%token <text> AC_GROUP AC_HELPER

/* ISA non-terminals */
%type <text> input isadec isadecbody formatdeclist instrdeclist ctordec
%type <text> formatdec instrdec ctordecbody asmdec decoderdec cyclesdec
%type <text> is_jumpdec is_branchdec conddec delaydec delayconddec behaviordec
%type <text> fieldinitlist fieldinit rangedec interval
%type <text> asmmaplist asmmapdec mapbodylist mapbody symbollist
%type <text> asmcomdec asmlcomdec
%type <text> pseudodec pseudobodylist pseudobody
%type <text> groupdeclist groupdec helperdec

/* ARCH Tokens */
%token <text> AC_ARCH
%token <text> AC_TLM_PORT
%token <text> AC_TLM_INTR_PORT
%token <text> AC_TLM2_PORT
%token <text> AC_TLM2_NB_PORT
%token <text> AC_TLM2_INTR_PORT
%token <text> AC_CACHE
%token <text> AC_ICACHE
%token <text> AC_DCACHE
%token <text> AC_MEM
%token <text> AC_PC
%token <text> AC_REGBANK
%token <text> AC_REG
%token <text> AC_STAGE
%token <text> AC_PIPE
%token <text> ARCH_CTOR
%token <text> AC_ISA
%token <text> AC_WORDSIZE
%token <text> AC_FETCHSIZE
%token <text> SET_ENDIAN
%token <text> BIND_TO
/*** Experimental feature. --Marilia
%token <text> AC_BUFFER AC_FETCHBUFFERSIZE
*/

/* ARCH non-terminals */
%type <text> archdec archdecbody stagedec pipedec declist worddec fetchdec archctordec
%type <text> storagedec storagelist memdec regbankdec cachedec cachenparm
%type <text> portdec intrportdec
%type <text> cachesparm cacheobjdec cacheobjdec1 regdec assignregparm assignwidth
/*** Experimental feature. --Marilia
%type <text> bufferdec fetchbufferdec
*/

/* General non-terminals */
%type <text> commalist id_list concatfields_list

%start input

%union
{
 char* text;
 int value;
}

/*************************************************/
/* ArchC Grammars for Instruction Set,           */
/* Micro-architecture and Stage  declarations    */
/*************************************************/
%%
input: /* empty string */
      {
       yyerror("No valid declaration provided");
       YYERROR;
      }
      | isadec
      {
       if (parse_error)
        YYERROR;
      }
      | archdec
      {
       if (parse_error)
        YYERROR;
      }
      ;

/*************************************************/
/* ArchC Grammar for Instruction Set declaration */
/*************************************************/
isadec: AC_ISA_UPPER LPAREN ID RPAREN LBRACE
      {
       descrp = ISA_D;
      }
      isadecbody RBRACE SEMICOLON {}
      ;

isadecbody:
      helperdec formatdeclist instrdeclist groupdeclist asmmaplist ctordec
      ;

/*******************************************/
/* Parsing helper declarations             */
/*******************************************/

helperdec: AC_HELPER BLOCK SEMICOLON
      {
       if (helper_contents)
        yywarn("Multiple ac_helper declarations. Ignored");
       else
       {
        helper_contents = $2;/*(char*) malloc(sizeof(char) * (1 + strlen($2)));*/
        /*strcpy(helper_contents, $2);*/
       }
      }
      | /* empty string */ {}
      ;

/*******************************************/
/* Parsing format declarations             */
/*******************************************/

formatdeclist: formatdeclist formatdec
      | /* empty string */ {}
      ;

formatdec: AC_FORMAT ID EQ STR SEMICOLON
      {
       if (descrp == ISA_D)
       {
        if (!add_format(&format_ins_list, &format_ins_list_tail, $2, $4, error_msg, 1))
         yyerror(error_msg);
        if (!add_symbol($2, INSTR_FMT, (void*) format_ins_list_tail))
         yyerror("Redefinition of %s", $2);
        format_num++;
       }
       else if (descrp == ARCH_D)
       {
        if (!add_format(&format_reg_list, &format_reg_list_tail, $2, $4, error_msg, 0))
         yyerror(error_msg);
        if (!add_symbol($2, REG_FMT, (void*) format_reg_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else
        yyerror("Internal Bug. Invalid description type. It should be ISA_D or ARCH_D");
      }
      ;

/************************************/
/* Parsing group declarations       */
/************************************/

groupdeclist: groupdeclist groupdec
      | /* empty string */ {}
      ;

groupdec: AC_GROUP ID LBRACE ID
      {
       int retval;

       current_group = add_group($2);
       if (!add_symbol($2, GEN_ID, (void*) group_list_tail))
        yyerror("Redefinition of %s", $2);
       retval = add_instr_ref($4, &current_group->instrs, error_msg);
       if (!retval)
        yyerror(error_msg);
       else if (retval == 2)
        yywarn(error_msg);
       list_type = INSTR_REF_L;
      }
      commalist RBRACE SEMICOLON
      ;

/************************************/
/* Parsing instruction declarations */
/************************************/

instrdeclist: instrdeclist instrdec
      | /* empty string */ {}
      ;

instrdec: AC_INSTR LT ID GT ID
      {
       /* Add to the instruction declaration list. */
       current_type = (char*) malloc(sizeof(char) * (strlen($3) + 1));
       strcpy(current_type,$3);
       if (find_instr($5))
	 yyerror("Redefinition of %s", $5);
       else {
	 if (!add_instr($5, current_type, &current_instr, error_msg))
	   yyerror(error_msg);
	 else {
	   if (!add_symbol($5, INSTR, (void*) instr_list_tail))
	     yyerror("Internal Bug. find_instr should have caught this");
	 }
       }
       list_type = INSTR_L;
      }
      commalist SEMICOLON
      {
       free(current_type);
      }
      ;


/**************************************************/
/* Rules to parse special acasm declarations      */
/**************************************************/
asmcomdec: AC_ASM_ASSEMBLER DOT AC_ASM_COMMENT LPAREN STR RPAREN SEMICOLON
      {
	if (!acpp_set_assembler_comment_chars($5, error_msg))
	  yyerror(error_msg);
      }
      ;
      
asmlcomdec: AC_ASM_ASSEMBLER DOT AC_ASM_LINE_COMMENT LPAREN STR RPAREN SEMICOLON
      {
	if (!acpp_set_assembler_line_comment_chars($5, error_msg))
	  yyerror(error_msg);
      }
      ;      
/**************************************************/
/* Rules to parse ac_asm_map declarations         */
/**************************************************/

asmmaplist: asmmaplist asmmapdec
      | {}
      ;

asmmapdec: AC_ASM_MAP ID
      {
       if (!acpp_asm_create_mapping_block($2, error_msg))
        yyerror(error_msg);
      }
      LBRACE mapbodylist RBRACE
      ;

mapbodylist: mapbodylist mapbody
      | mapbody
      ;

mapbody: STR
      {
       if (!acpp_asm_add_mapping_symbol($1, error_msg))
        yyerror(error_msg);
      }
      symbollist EQ INT SEMICOLON
      {
       if (!acpp_asm_add_symbol_value($5, $5, error_msg))
        yyerror(error_msg);
      }
      | STR LBRACK INT PP INT RBRACK EQ LBRACK INT PP INT RBRACK SEMICOLON
      {
       if (!acpp_asm_add_mapping_symbol_range($1, NULL, $3, $5,  error_msg))
        yyerror(error_msg);
       if (!acpp_asm_add_symbol_value($9, $11, error_msg))
        yyerror(error_msg);
      }
      | LBRACK INT PP INT RBRACK STR EQ LBRACK INT PP INT RBRACK SEMICOLON
      {
       if (!acpp_asm_add_mapping_symbol_range(NULL, $6, $2, $4,  error_msg))
        yyerror(error_msg);
       if (!acpp_asm_add_symbol_value($9, $11, error_msg))
        yyerror(error_msg);
      }
      | STR LBRACK INT PP INT RBRACK STR EQ LBRACK INT PP INT RBRACK SEMICOLON
      {
       if (!acpp_asm_add_mapping_symbol_range($1, $7, $3, $5,  error_msg))
        yyerror(error_msg);
       if (!acpp_asm_add_symbol_value($10, $12, error_msg))
        yyerror(error_msg);
      }
      ;

symbollist: COMMA STR
      {
       if (!acpp_asm_add_mapping_symbol($2, error_msg))
        yyerror(error_msg);
      }
      symbollist
      | {}
      ;

/**************************************************/
/* Rules to parse the ISA constructor declaration */
/**************************************************/

ctordec: ISA_CTOR LPAREN ID RPAREN LBRACE ctordecbody RBRACE SEMICOLON
      ;

ctordecbody: asmdec ctordecbody
      | decoderdec ctordecbody
      | cyclesdec ctordecbody
      | rangedec ctordecbody
      | is_jumpdec ctordecbody
      | is_branchdec ctordecbody
      | conddec ctordecbody
      | delaydec ctordecbody
      | delayconddec ctordecbody
      | behaviordec ctordecbody
      | pseudodec ctordecbody
      | asmcomdec ctordecbody
      | asmlcomdec ctordecbody
      | /* empty string */ {}
      ;

asmdec: ID DOT SET_ASM LPAREN STR
      {
       // search for the instruction and store it in 'current_instr' for future use
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
       {
        if (current_instr->asm_str == NULL)
        {
         current_instr->asm_str = (char*) malloc(sizeof(char) * (strlen($5) + 1));
         strcpy(current_instr->asm_str, $5);
        }
        if (current_instr->mnemonic == NULL)
        {
         char* aux = $5;
         while (*aux != ' ' && *aux != '\0')
          aux++;
         current_instr->mnemonic =(char*) malloc(sizeof(char) * (aux - $5 + 1));
         memcpy(current_instr->mnemonic, $5, aux - $5);
         current_instr->mnemonic[aux - $5] = '\0';
        }
        // search for the insn format and store it in 'current_format' for future use
        current_format = find_format(current_instr->format);
        if (current_format == NULL)
         yyerror("Invalid format");
        acpp_asm_new_insn();
        if (!acpp_asm_parse_asm_string($5, 0, error_msg))
        {
         if (!force_setasm_syntax)
         {
          if (!is_in_old_setasm_mode)
          {
           yywarn("Entering old set_asm() syntax compatible mode");
           is_in_old_setasm_mode = 1;
          }
         }
         else
          yyerror(error_msg);
        }
       }
      }
      id_list RPAREN SEMICOLON
      {
       if (!acpp_asm_end_insn(current_instr, error_msg))
       {
        if (!force_setasm_syntax)
        {
         if (!is_in_old_setasm_mode)
         {
          yywarn("Entering old set_asm() syntax compatible mode");
          is_in_old_setasm_mode = 1;
         }
        }
        else
         yyerror(error_msg);
       }
      }
      ;

id_list: COMMA ID EQ INT
      {
       force_setasm_syntax = 1;
       if (!acpp_asm_parse_const_asm_argument(current_format, $2, NULL, $4, NULL, error_msg))
        yyerror(error_msg);
      }
      id_list
      | COMMA ID EQ ID DOT MAP_TO LPAREN STR RPAREN
      {
       force_setasm_syntax = 1;
       if (!acpp_asm_parse_const_asm_argument(current_format, $2, $4, 0, $8, error_msg))
        yyerror(error_msg);
      }
      id_list
      | COMMA ID EQ STR
      {
       force_setasm_syntax = 1;
       if (!acpp_asm_parse_const_asm_argument(current_format, $2, NULL, 0, $4, error_msg))
        yyerror(error_msg);
      }
      id_list
      | COMMA concatfields_list
      {
       force_setasm_syntax = 1;
      }
      id_list
      | {}
      ;
      
concatfields_list: concatfields_list PLUS ID
      {	
        if (!acpp_asm_parse_asm_argument(current_format, $3, 1, error_msg))
	  yyerror(error_msg);
      }      
      | ID
      {	
        if (!acpp_asm_parse_asm_argument(current_format, $1, 0, error_msg))
	   yyerror(error_msg);
      }
      ;

decoderdec: ID DOT SET_DECODER LPAREN ID EQ INT
      {
       /* Middle rule action to look for the instr. */
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
       {
        if (!add_dec_list(current_instr, $5, $7, error_msg))
         yyerror(error_msg);
       }
      } /* End of action */
      fieldinitlist RPAREN SEMICOLON
      ;

cyclesdec: ID DOT SET_CYCLES LPAREN INT RPAREN SEMICOLON
      {
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("undeclared instruction: %s", $1);
       else
       {
        current_instr->cycles = $5;
        HaveMultiCycleIns = 1;
       }
      }
      ;

rangedec: ID DOT CYCLE_RANGE LPAREN INT
      /* action */
      {
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
       {
        current_instr->min_latency = $5;
       }
       HaveCycleRange = 1;
      }
      /* continue rule ... */
      interval RPAREN SEMICOLON
      ;

pseudodec: PSEUDO_INSTR LPAREN STR RPAREN
      {
       acpp_asm_new_insn();
       if (!acpp_asm_parse_asm_string($3, 0, error_msg))
        yyerror(error_msg);
       // don't change the order
       acpp_asm_new_pseudo();
      }
      LBRACE pseudobodylist RBRACE
      {
       if (!acpp_asm_end_insn(NULL, error_msg))
        yyerror(error_msg);
      }
      ;

pseudobodylist: pseudobodylist pseudobody
      | pseudobody
      ;

pseudobody: STR SEMICOLON
      {
       if (!acpp_asm_add_pseudo_member($1, error_msg))
        yyerror(error_msg);
      }
      ;

interval: COMMA INT
      /* current_instr is now pointing to the instruction given by ID field in the rule above */
      {
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
        current_instr->max_latency = $2;
      }
      | /* empty string */
      {
       if (current_instr == NULL)
        yyerror("Internal error");
       else
        current_instr->max_latency = current_instr->min_latency;
      }
      ;

fieldinitlist: COMMA fieldinit fieldinitlist {}
      | /* empty string */ {}
      ;

fieldinit: ID EQ INT
      {
       /* Adding field to the end of the decoding sequence list */
       if (!add_dec_list(current_instr, $1, $3, error_msg))
        yyerror(error_msg);
      }
      ;

is_jumpdec: ID DOT IS_JUMP BLOCK SEMICOLON
      {
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
       {
        get_control_flow_struct(current_instr)->target = $4;
        get_control_flow_struct(current_instr)->cond = "1";
       }
      }
      ;

is_branchdec: ID DOT IS_BRANCH BLOCK SEMICOLON
      {
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
        get_control_flow_struct(current_instr)->target = $4;
      }
      ;

conddec: ID COND BLOCK SEMICOLON
      {
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
       {
        get_control_flow_struct(current_instr)->cond = $3;
        ControlInstrInfoLevel = 2;
       }
      }
      ;

delaydec: ID DOT DELAY BLOCK SEMICOLON
      {
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
       {
        get_control_flow_struct(current_instr)->delay_slot = strtol($4,0,0);
        get_control_flow_struct(current_instr)->delay_slot_cond = "1";
        if (ControlInstrInfoLevel == 0)
         ControlInstrInfoLevel = 1;
       }
      }
      ;

delayconddec: ID DOT DELAY_COND BLOCK SEMICOLON
      {
       current_instr = find_instr($1);
       if (current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
        get_control_flow_struct(current_instr)->delay_slot_cond = $4;
      }
      ;

behaviordec: ID DOT BEHAVIOR BLOCK SEMICOLON
      {
       current_instr = find_instr($1);
       if(current_instr == NULL)
        yyerror("Undeclared instruction: %s", $1);
       else
        get_control_flow_struct(current_instr)->action = $4;
      }
      ;

/******************************************************/
/* ArchC Grammar for ARCH  declarations               */
/******************************************************/
archdec: AC_ARCH LPAREN ID RPAREN LBRACE
      {
       /* Middle rule action to start emitting ARCH module declaration. */
       project_name = (char*) malloc(sizeof(char) * (strlen($3) + 1));
       upper_project_name = (char*) malloc(sizeof(char) * (strlen($3) + 1));
       strcpy(project_name, $3);
       strcpy(upper_project_name, $3);
       str_upper(upper_project_name);
       descrp = ARCH_D;
       helper_contents = NULL;
       /* Initialisation section */
       /* local variables */
       current_type = NULL;
       current_instr = NULL;
       current_format = NULL;
       current_unit = BYTE;
       current_pipe = NULL;
       cache_type = CACHE;
       list_type = MEM_L; /* it does not matter */
       error_msg[0] = '\0';
       is_in_old_setasm_mode = 0;
       parse_error = 0;
       init_core_actions();
      } /* End of action */
      archdecbody RBRACE SEMICOLON
      {
       /* Closing module declaration. */
      } /* End of action */
      ;

archdecbody: declist archctordec
      ;

/*******************************************************/
/* Rules to parse the ARCH elements declaration.       */
/* Except ac_format, which is already defined above.   */
/*******************************************************/

declist: storagedec declist
      | formatdec declist
      | stagedec declist
      | worddec declist
      | fetchdec declist
/*** Experimental feature. --Marilia
      | fetchbufferdec declist
*/
      | pipedec declist
      | /* empty string */ {}
      ;

/*******************************************************/
/* Rules to parse the ARCH storage device declaration */
/*******************************************************/

/* Units may be present in memories and caches declarations */
unit: ID
      {
       /* Identifying unit */
       if (!strcmp($1, "K") || !strcmp($1, "k"))
        current_unit = KBYTE;
       else if (!strcmp($1, "M") || !strcmp($1, "m"))
        current_unit = MBYTE;
       else if (!strcmp($1, "G") || !strcmp($1, "g"))
        current_unit = GBYTE;
       else
        yyerror("Invalid storage unit. It should be \"K\", \"M\", \"G\"");
      }
      | /* empty */
      {
       current_unit = BYTE;
      }
      ;

/* Storage Devices Declarations */
storagedec: cachedec
      | memdec
      | regbankdec
      | regdec
      | portdec
      | intrportdec
/*** Experimental feature. --Marilia
      | bufferdec
*/
      ;

/* Generic Memory Declaration */
memdec: AC_MEM ID COLON INT unit
      {
       /* Including device in storage list. */
       if (!add_storage($2, ($4 * current_unit), (ac_sto_types) MEM, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $2);
       list_type = MEM_L;
      }
      storagelist SEMICOLON
      ;

/* Port declaration */
portdec: AC_TLM_PORT ID COLON INT unit
      {
       /* Including port in storage list */
       if (!add_storage($2, ($4 * current_unit), (ac_sto_types) TLM_PORT, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $2);
       list_type = TLM_PORT_L;
       HaveTLMPorts = 1;
      }
      storagelist SEMICOLON
      ;
      
      
      /* Port declaration */
portdec: AC_TLM2_PORT ID COLON INT unit
      {
       /* Including port in storage list */
       if (!add_storage($2, ($4 * current_unit), (ac_sto_types) TLM2_PORT, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $2);
       list_type = TLM2_PORT_L;
       HaveTLM2Ports = 1;
      }
      storagelist SEMICOLON
      ;

/* NB Port declaration */
portdec: AC_TLM2_NB_PORT ID COLON INT unit
      {
       /* Including port in storage list */
       if (!add_storage($2, ($4 * current_unit), (ac_sto_types) TLM2_NB_PORT, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $2);
       list_type = TLM2_PORT_L;
       HaveTLM2NBPorts = 1;
      }
      storagelist SEMICOLON
      ;
      
      
      

/* Interruption Port declaration */
intrportdec: AC_TLM_INTR_PORT ID
      {
       /* Including port in storage list */
       if (!add_storage($2, 0, (ac_sto_types) TLM_INTR_PORT, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $2);
       list_type = TLM_INTR_PORT_L;
       HaveTLMIntrPorts = 1;
      }
      storagelist SEMICOLON
      ;

/* Interruption Port declaration */
intrportdec: AC_TLM2_INTR_PORT ID
      {
       /* Including port in storage list */
       if (!add_storage($2, 0, (ac_sto_types) TLM2_INTR_PORT, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $2);
       list_type = TLM2_INTR_PORT_L;
       HaveTLM2IntrPorts = 1;
      }
      storagelist SEMICOLON
      ;


/* Cache Declaration */
cachedec: AC_CACHE
      {
       cache_type = (ac_sto_types) CACHE;
      }
      cacheobjdec
      | AC_ICACHE
      {
       cache_type = (ac_sto_types) ICACHE;
      }
      cacheobjdec
      | AC_DCACHE
      {
       cache_type = (ac_sto_types) DCACHE;
      }
      cacheobjdec
      ;

/* Cache object declaration */
cacheobjdec: ID COLON INT unit /* Generic cache. Just a container for data */
      {
       /* Including device in storage list. */
       if (!add_storage($1, ($3 * current_unit), cache_type, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($1, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $1);
       list_type = CACHE_L;
      }
      storagelist SEMICOLON
      /* INSTANCES of ArchC cache simulation class (ac_cache). */
      /* Parameters for ac_cache instantiation may be interdependent, for example,
         replacement strategy only makes sense for non-directed-mapped caches. So, put all
         given parameters in a list and let the simulator generator take care of this analisys.
         We may have 5 or 6 parameters.*/
             /* associativity        nblocks         blocksize        replacestrtgy/writemethod          */
      | ID LPAREN cachesparm COMMA cachenparm COMMA cachenparm COMMA cachesparm COMMA cachesparm cacheobjdec1
      {
       if (!add_storage($1, 0, cache_type, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($1, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $1);
      }
      ;

/* ac_cache object declarations with 4 or 5 parameters */
                 /* writemethod */
cacheobjdec1: COMMA cachesparm RPAREN SEMICOLON
      |  RPAREN SEMICOLON
      ;

/* numeric parameter */
cachenparm: INT
      {
       add_parms(NULL, $1);
      }
//    | /* empty string */ {}
      ;

/* string parameter */
cachesparm: STR
      {
       add_parms($1, 0);
      }
//    | /* empty string */ {}
      ;

/* Register Bank Declaration */
regbankdec: AC_REGBANK assignwidth ID COLON INT
      {
       /* Including device in storage list. */
       if (!add_storage($3, $5, (ac_sto_types) REGBANK, NULL, error_msg))
        yyerror(error_msg);
       if (!add_symbol($3, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $3);
       list_type = REGBANK_L;
      }
      storagelist SEMICOLON
      ;

/* Single Register Declaration */
regdec: AC_REG assignregparm ID
      {
       /* Including device in storage list. */
       if (!add_storage($3, 0, (ac_sto_types) REG, current_type, error_msg))
        yyerror(error_msg);
       if (!add_symbol($3, GEN_ID, (void*) storage_list_tail))
        yyerror("Redefinition of %s", $3);
       list_type = REG_L;
      }
      commalist SEMICOLON
      {
       if (current_type)
        free(current_type);
       current_type = NULL;
      }
      ;

/*** Experimental feature. --Marilia
bufferdec: AC_BUFFER LT INT GT ID SEMICOLON // TODO: Something! //
      ;
*/

/* Managing a list of devices declaraded in the same line
   with ac_cache and ac_regbank */
storagelist: COMMA ID COLON INT
      {
       /* Including device in storage list. */
       if (list_type == CACHE_L)
       {
        if (!add_storage($2, ($4 * current_unit), (ac_sto_types) CACHE, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else if (list_type == REGBANK_L)
       {
        if (!add_storage($2, $4, (ac_sto_types) REGBANK, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else if (list_type == MEM_L)
       {
        if (!add_storage($2, ($4 * current_unit), (ac_sto_types) MEM, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else if (list_type == TLM_PORT_L)
       {
        if (!add_storage($2, ($4 * current_unit), (ac_sto_types) TLM_PORT, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else if (list_type == TLM2_PORT_L)
       {
        if (!add_storage($2, ($4 * current_unit), (ac_sto_types) TLM2_PORT, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else if (list_type == TLM2_NB_PORT_L)
       {
        if (!add_storage($2, ($4 * current_unit), (ac_sto_types) TLM2_NB_PORT, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else if (list_type == TLM_INTR_PORT_L)
       {
        if (!add_storage($2, $4, (ac_sto_types) TLM_INTR_PORT, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
        else if (list_type == TLM2_INTR_PORT_L)
       {
        if (!add_storage($2, $4, (ac_sto_types) TLM2_INTR_PORT, NULL, error_msg))
         yyerror(error_msg);
        if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
         yyerror("Redefinition of %s", $2);
       }
       else
       {
        /* Should never enter here. */
        yyerror("Internal Bug. Invalid list type. It should be CACHE_L, REGBANK_L, MEM_L, TLM_PORT_L, TLM2_INTR_PORT_L or TLM_INTR_PORT_L");
       }
      }
      | /* empty string */ {}
      ;

/* Assign a format to a register must be optional,
   that is why we need a non-terminal and a new rule. */
assignregparm: LT ID GT
      {
       /* Assign a format to storage device. */
       /* capturing format name */
       current_type = (char*) malloc(sizeof(char) * (strlen($2) + 1));
       strcpy(current_type, $2);
       HaveFormattedRegs = 1;
       reg_width = 0;
      }
      | LT INT GT
      {
       /* Assign a width to a register, instead of a format. */
       current_type = NULL;
       reg_width = $2;
      }
      | /* empty string */
      {
       current_type = NULL;
       reg_width = 0;
      }
      ;

/* Assign a width different from wordsize to registers in a register bank must be optional,
   that is why we need a non-terminal and a new rule. */
assignwidth: LT INT GT
      {
       /* Getting register width. */
       reg_width = $2;
      }
      | /* empty string */
      {
       reg_width = 0;
      }
      ;

/****************************************************/
/* Rules to parse the ARCH stage declaration       */
/****************************************************/

stagedec: AC_STAGE ID
      {
       ac_stg_list* current_stage;
       char* curr_stg_name;

       /* Create an anonymous pipe if it does not already exist. */
       if (!anonymous_pipe)
       {
        anonymous_pipe = add_pipe("");
        current_stage = add_stage($2, &anonymous_pipe->stages);
        curr_stg_name = (char*) malloc(sizeof(char) * (2 + strlen($2)));
        strcpy(curr_stg_name, "_");
        strcat(curr_stg_name, $2);
        if (!add_symbol(curr_stg_name, GEN_ID, (void*) current_stage))
         yyerror("Redefinition of %s", $2);
        free(curr_stg_name);
        stage_list = anonymous_pipe->stages;
       }
       else
       {
        current_stage = add_stage($2, &anonymous_pipe->stages);
        curr_stg_name = (char*) malloc(sizeof(char) * (2 + strlen($2)));
        strcpy(curr_stg_name, "_");
        strcat(curr_stg_name, $2);
        if (!add_symbol(curr_stg_name, GEN_ID, (void*) current_stage))
         yyerror("Redefinition of %s", $2);
        free(curr_stg_name);
       }
       /* Add to the stage list. */
       list_type = STAGE_L;
      }
      commalist SEMICOLON
      ;

/****************************************************/
/* Rules to parse the ARCH pipeline declaration     */
/****************************************************/
pipedec: AC_PIPE ID EQ LBRACE ID
      {
       ac_stg_list* current_stage;
       char* curr_stg_name;

       /* Indicating to commalist that this is a pipe declaration */
       list_type = PIPE_L;
       /* Adding to the pipe list */
       current_pipe = add_pipe($2);
       if (!add_symbol($2, GEN_ID, (void*) current_pipe))
        yyerror("Redefinition of %s", $2);
       /* Adding the first stage to the pipe's stage list */
       current_stage = add_stage($5, &current_pipe->stages);
       curr_stg_name = (char*) malloc(sizeof(char) * (2 + strlen($2) + strlen($5)));
       strcpy(curr_stg_name, $2);
       strcat(curr_stg_name, "_");
       strcat(curr_stg_name, $5);
       if (!add_symbol(curr_stg_name, GEN_ID, (void*) current_stage))
        yyerror("Redefinition of %s inside pipeline %s", $5, $2);
       free(curr_stg_name);
      }
      commalist RBRACE SEMICOLON
      ;

/*******************************************************************************/
/* Rules to parse the ARCH wordsize, fetchsize and fetchbuffersize declaration */
/*******************************************************************************/
worddec: AC_WORDSIZE INT SEMICOLON
      {
       /* Declaring wordsize for the architecture */
       wordsize = $2;
      }
      ;

fetchdec: AC_FETCHSIZE INT SEMICOLON
      {
       /* Declaring fetchsize for the architecture */
       fetchsize = $2;
      }
      ;

/*** Experimental feature. --Marilia
fetchbufferdec: AC_FETCHBUFFERSIZE INT SEMICOLON
      {
       fetchbuffersize = $2;
      }
      ;
*/

/****************************************************/
/* Rules to parse the ARCH constructor declaration */
/****************************************************/
archctordec: ARCH_CTOR LPAREN ID RPAREN LBRACE archctordecbody RBRACE SEMICOLON
      ;

archctordecbody: AC_ISA LPAREN STR RPAREN SEMICOLON archctordecbody
      {
       isa_filename = (char*) malloc(sizeof(char) * (strlen($3) + 1));
       strcpy(isa_filename ,$3);
      }
      | SET_ENDIAN LPAREN STR RPAREN SEMICOLON archctordecbody
      {
       if (!strcmp($3, "little") || !strcmp($3, "LITTLE"))
        ac_tgt_endian = 0;
       else if (!strcmp($3, "big") || !strcmp($3, "BIG"))
        ac_tgt_endian = 1;
       else
       {
        yyerror("Invalid endian: \"%s\"", $3);
       }
      }
      | ID DOT BIND_TO LPAREN ID RPAREN SEMICOLON archctordecbody
      {
        ac_sto_list* plow;
        ac_sto_list* pstorage;

        /* Warning acpp that a memory hierarchy was declared */
        HaveMemHier = 1;
        /* Looking for the low level device */
        plow = find_storage($1);
        if (plow == NULL)
         yyerror("Undeclared storage device: %s", $1);
        /* Looking for the high level device */
        pstorage = find_storage($5);
        if (pstorage == NULL)
         yyerror("Undeclared storage device: %s", $5);
        plow->higher = pstorage;
        // Updating hierarchy level, from this level to the top.
        for (pstorage = plow; pstorage->higher != NULL; pstorage = pstorage->higher)
         pstorage->higher->level = pstorage->level + 1;
      }
      | /* empty string */ {}
      ;

/*********************************************************/
/* Rules used into several other rules to help parsing   */
/* several ArchC constructions                           */
/*********************************************************/
commalist: COMMA ID
      {
       ac_stg_list* current_stage;
       char* curr_stg_name;
       int retval;

       switch (list_type)
       {
        case INSTR_L:
	 if (find_instr($2))
	  yyerror("Redefinition of %s", $2);
	 else {
          if (!add_instr($2, current_type, &current_instr, error_msg)) /* Add to instruction list. */
           yyerror(error_msg);
	  else {
           if (!add_symbol($2, INSTR, (void*) instr_list_tail))
            yyerror("Internal Bug. find_instr should have caught this");
          }
	 }
	 break;
        case STAGE_L:
         /* This case is reached when a pipe declaration like
            ac_stages ST1, ST2, ST3;
            is used. So, add stage to the anonymous pipe's stage list */
         current_stage = add_stage($2, &anonymous_pipe->stages);
         curr_stg_name = (char*) malloc(sizeof(char) * (2 + strlen($2)));
         strcpy(curr_stg_name, "_");
         strcat(curr_stg_name, $2);
         if (!add_symbol(curr_stg_name, GEN_ID, (void*) current_stage))
          yyerror("Redefinition of %s", $2);
         free(curr_stg_name);
         break;
        case REG_L:
         if (!add_storage($2, 0, (ac_sto_types) REG, current_type, error_msg)) /* Add to reg list. */
          yyerror(error_msg);
         if (!add_symbol($2, GEN_ID, (void*) storage_list_tail))
          yyerror("Redefinition of %s", $2);
         break;
        case PIPE_L:
         /* In this case, IDs represent stage names, so
            add stage to the pipe's stage list. */
         current_stage = add_stage($2, &current_pipe->stages);
         curr_stg_name = (char*) malloc(sizeof(char) * (2 + strlen(current_pipe->name) + strlen($2)));
         strcpy(curr_stg_name, current_pipe->name);
         strcat(curr_stg_name, "_");
         strcat(curr_stg_name, $2);
         if (!add_symbol(curr_stg_name, GEN_ID, (void*) current_stage))
          yyerror("Redefinition of %s inside pipeline %s", $2, current_pipe->name);
         free(curr_stg_name);
         break;
        case INSTR_REF_L:
         retval = add_instr_ref($2, &current_group->instrs, error_msg);
         if (!retval)
          yyerror(error_msg);
         else if (retval == 2)
          yywarn(error_msg);
         break;
        default:
         /* Should never enter here. */
         yyerror("Internal Bug. Unidentified list type");
         break;
       }
      }
      commalist
      | /* empty string */ {}
      ;

%%

/**********************

   File Structure
   ==============
%{
C declarations
%}

Bison declarations

%%
Grammar rules
%%

Additional C code
***********************/

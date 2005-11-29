/*  ArchC Language Sintax for GNU Flex
    Copyright (C) 2002-2004  The ArchC Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

%{
#include <stdlib.h>
#include <string.h>
#include "archc.parser.h"

#define MAX_STR_CONST  256
#define MAX_CODE_CONST  256
#define MAX_BLOCK_CONST  4096
#define DEBUG_LEX 0
%}

DIGIT    [0-9]
ID       [a-zA-Z][_a-zA-Z0-9]*
HEXA_DIGIT [0-9A-Fa-f]

%x COMMENT
%x COMMENT1
%x STRING
%x BLOCK_START BLOCK_STATE
%x HEXA
%x ARCH

%option noyywrap

%%

   char string_buf[MAX_STR_CONST];
   char *string_buf_ptr;
   char block_buf[MAX_BLOCK_CONST];
   char *block_buf_ptr;
   int brace_level=0;
   int paren_level=0;
   int return_state;
   extern unsigned line_num;
 /***************************************************/
 /* Regular expressions for general tokens in ArchC */
 /***************************************************/

<INITIAL,ARCH>{DIGIT}+ {
     #if DEBUG_LEX
            printf( "An integer: %s (%d)\n", yytext,
                    atoi( yytext ) );
     #endif
	    yylval.value = atoi( yytext );
	    return INT;
   }


<INITIAL,ARCH>","    { 
     #if DEBUG_LEX
	    printf( "COMMA: %s\n", yytext );
     #endif
            return COMMA; 
   }

<INITIAL,ARCH>";"   {
     #if DEBUG_LEX
              printf( "SEMICOLON: %s\n", yytext );
     #endif
            return SEMICOLON; 

   }

<INITIAL,ARCH>":"   {
     #if DEBUG_LEX
              printf( "COLON: %s\n", yytext );
     #endif
            return COLON; 

   }


<INITIAL,ARCH>"."   { 
     #if DEBUG_LEX
              printf( "DOT: %s\n", yytext );
     #endif
              return DOT; 

   }

<INITIAL,ARCH>"="   {
     #if DEBUG_LEX
              printf( "EQUAL: %s\n", yytext );
     #endif
              return EQ; 

   }

<INITIAL,ARCH>"<"   {
     #if DEBUG_LEX
              printf( "LT: %s\n", yytext );
     #endif
              return LT; 

   }

<INITIAL,ARCH>">"   {
     #if DEBUG_LEX
              printf( "GT: %s\n", yytext );
     #endif
              return GT; 

   }

<INITIAL,ARCH>"("   {
     #if DEBUG_LEX
              printf( "LPAREN: %s\n", yytext );
     #endif
              return LPAREN; 
   }

<INITIAL,ARCH>")"   {
     #if DEBUG_LEX
              printf( "RPAREN: %s\n", yytext );
     #endif
              return RPAREN; 

   }

<INITIAL>"["   {
     #if DEBUG_LEX
              printf( "LBRACK: %s\n", yytext );
     #endif
              return LBRACK; 

   }

<INITIAL>"]"   {
     #if DEBUG_LEX
              printf( "RBRACK: %s\n", yytext );
     #endif
              return RBRACK; 

   }

<INITIAL,ARCH>"{"    {
     #if DEBUG_LEX
              printf( "LBRACE: %s\n", yytext );
     #endif
              return LBRACE; 

   }

<INITIAL,ARCH>"}"    {
     #if DEBUG_LEX
              printf( "RBRACE: %s\n", yytext );
     #endif
              return RBRACE; 

   }



<INITIAL>"0x"  BEGIN(HEXA);

<INITIAL,ARCH>\"   {
             string_buf_ptr = string_buf; 
	     return_state = YY_START;
             BEGIN(STRING);
   }

<INITIAL,ARCH>"/*"   {
     #if DEBUG_LEX
              printf( "Entering comment: \n" );
     #endif
	     return_state = YY_START;
			 BEGIN(COMMENT);
   }

<INITIAL,ARCH>"//"   {
     #if DEBUG_LEX
              printf( "Entering comment1: \n" );
     #endif
	     return_state = YY_START;
			 BEGIN(COMMENT1);
   }

<INITIAL,ARCH>".."   {
     #if DEBUG_LEX
              printf( "PP \n" );
     #endif
	      return PP;
   }

<INITIAL,ARCH>\n     { 
     line_num++;
     #if DEBUG_LEX
             printf( "New line: %d\n",line_num);
     #endif
   }

<INITIAL,ARCH>[" "\t\r]     /* eat up spaces */   { 
     #if DEBUG_LEX
             printf( "Eating space: <%s>\n",yytext);
     #endif
   }

 /***************************************************/
 /* Regular expressions for ArchC ISA reserved words*/
 /***************************************************/

<INITIAL>"AC_ISA"   { 
     #if DEBUG_LEX
              printf( "AC_ISA: %s\n", yytext );
     #endif

				return AC_ISA_UPPER; 

   }

<INITIAL>"ISA_CTOR"   {
     #if DEBUG_LEX
              printf( "ISA_CTOR: %s\n", yytext );
     #endif
              return ISA_CTOR; 

   }

<INITIAL>"set_decoder"   {
     #if DEBUG_LEX
              printf( "SET_DECODER: %s\n", yytext );
     #endif
              return SET_DECODER; 

   }

<INITIAL>"set_asm"   {
     #if DEBUG_LEX
              printf( "SET_ASM: %s\n", yytext );
      #endif
             return SET_ASM; 

   }

<INITIAL>"set_cycles"   {
     #if DEBUG_LEX
              printf( "SET_CYCLES: %s\n", yytext );
      #endif
             return SET_CYCLES; 

   }

<INITIAL>"cycle_range"   {
     #if DEBUG_LEX
              printf( "CYCLE_RANGE: %s\n", yytext );
      #endif
             return CYCLE_RANGE; 

   }

<INITIAL>"ac_format"   {
     #if DEBUG_LEX
              printf( "AC_FORMAT: %s\n", yytext );
      #endif
             return AC_FORMAT; 

   }

<INITIAL>"ac_instr"   { 
     #if DEBUG_LEX
              printf( "AC_INSTR: %s\n", yytext );
     #endif
             return AC_INSTR; 
 
   }

<INITIAL>"ac_asm_map" {
     #if DEBUG_LEX
              printf( "AC_ASM_MAP: %s\n", yytext );
     #endif
             return AC_ASM_MAP;  
   }

<INITIAL>"pseudo_instr" {
     #if DEBUG_LEX
              printf( "PSEUDO_INSTR: %s\n", yytext );
     #endif
             return PSEUDO_INSTR;  
   }

<INITIAL>"is_jump"   {
     #if DEBUG_LEX
             printf( "IS_JUMP: %s\n", yytext );
     #endif
             return_state = YY_START;
             BEGIN(BLOCK_START);
             return IS_JUMP; 

   }

<INITIAL>"is_branch"   {
     #if DEBUG_LEX
             printf( "IS_BRANCH: %s\n", yytext );
     #endif
             return_state = YY_START;
             BEGIN(BLOCK_START);
             return IS_BRANCH; 

   }

<INITIAL>".cond"   {
     #if DEBUG_LEX
             printf( "COND: %s\n", yytext );
     #endif
             return_state = YY_START;
             BEGIN(BLOCK_START);
             return COND; 

   }

<INITIAL>"delay"   {
     #if DEBUG_LEX
             printf( "DELAY: %s\n", yytext );
     #endif
             return_state = YY_START;
             BEGIN(BLOCK_START);
             return DELAY; 

   }

<INITIAL>"delay_cond"   {
     #if DEBUG_LEX
             printf( "DELAY_COND: %s\n", yytext );
     #endif
             return_state = YY_START;
             BEGIN(BLOCK_START);
             return DELAY_COND; 

   }

<INITIAL>"behavior"   {
     #if DEBUG_LEX
             printf( "BEHAVIOR: %s\n", yytext );
     #endif
             return_state = YY_START;
             BEGIN(BLOCK_START);
             return BEHAVIOR; 

   }

<INITIAL>"AC_ARCH"   { 
     /* Changing to ARCH state */
     #if DEBUG_LEX
              printf( "AC_ARCH: %s\n", yytext );
     #endif
	      line_num = 1;
 	      BEGIN(ARCH);
              return AC_ARCH; 

   }

 /******************************************************/
 /* Regular expressions for ARchC ARCH reserved words */
 /******************************************************/


<ARCH>"ARCH_CTOR"   {
     #if DEBUG_LEX
              printf( "ARCH_CTOR: %s\n", yytext );
     #endif
              return ARCH_CTOR; 

   }


<ARCH>"ac_cache"   {
     #if DEBUG_LEX
              printf( "AC_CACHE: %s\n", yytext );
      #endif
             return AC_CACHE; 

   }

<ARCH>"ac_icache"   {
     #if DEBUG_LEX
              printf( "AC_ICACHE: %s\n", yytext );
      #endif
             return AC_ICACHE; 

   }
<ARCH>"ac_dcache"   {
     #if DEBUG_LEX
              printf( "AC_DCACHE: %s\n", yytext );
      #endif
             return AC_DCACHE; 

   }


<ARCH>"ac_mem"   {
     #if DEBUG_LEX
              printf( "AC_MEM: %s\n", yytext );
      #endif
             return AC_MEM; 

   }

<ARCH>"ac_pc"   {
     #if DEBUG_LEX
              printf( "AC_PC: %s\n", yytext );
      #endif
             return AC_PC; 

   }

<ARCH>"ac_isa"   {
     #if DEBUG_LEX
              printf( "ac_isa: %s\n", yytext );
      #endif
             return AC_ISA; 

   }

<ARCH>"ac_reg"   { 
     #if DEBUG_LEX
              printf( "AC_REG: %s\n", yytext );
     #endif
             return AC_REG; 
 
   }

<ARCH>"ac_regbank"   { 
     #if DEBUG_LEX
              printf( "AC_REGBANK: %s\n", yytext );
     #endif
             return AC_REGBANK; 
 
   }

<ARCH>"ac_format"   {
     #if DEBUG_LEX
              printf( "AC_FORMAT FOR ARCH: %s\n", yytext );
      #endif
             return AC_FORMAT; 

   }

<ARCH>"ac_wordsize"   { 
     #if DEBUG_LEX
              printf( "AC_WORDSIZE: %s\n", yytext );
     #endif
             return AC_WORDSIZE; 
 
   }

<ARCH>"ac_fetchsize"   { 
     #if DEBUG_LEX
              printf( "AC_FETCHSIZE: %s\n", yytext );
     #endif
             return AC_FETCHSIZE; 
 
   }

<ARCH>"ac_stage"   { 
     #if DEBUG_LEX
              printf( "AC_STAGE: %s\n", yytext );
     #endif
             return AC_STAGE; 
 
   }

<ARCH>"ac_pipe"   { 
     #if DEBUG_LEX
              printf( "AC_PIPE: %s\n", yytext );
     #endif
             return AC_PIPE; 
 
   }

<ARCH>"set_endian"   { 
     #if DEBUG_LEX
              printf( "SET_ENDIAN: %s\n", yytext );
     #endif
             return SET_ENDIAN; 
 
   }

<ARCH>"bindTo"   { 
     #if DEBUG_LEX
              printf( "BIND_TO: %s\n", yytext );
     #endif
             return BIND_TO; 
 
   }

<ARCH>"bindsTo"   { 
     #if DEBUG_LEX
              printf( "BIND_TO: %s\n", yytext );
     #endif
             return BIND_TO; 
 
   }

<INITIAL,ARCH>{ID}   { 
     #if DEBUG_LEX
              printf( "An identifier: %s\n", yytext );
     #endif
              yylval.text = (char*)malloc(strlen(yytext)+1);
              strcpy(yylval.text,yytext);
              return ID; 

   }

<INITIAL,ARCH><<EOF>>   { 
             BEGIN(INITIAL);
             yyterminate();
   } 

.  { 
             printf( "ArchC Unmatched input at line %d: <%s>\n", line_num, yytext);
   }

 /********************************************************/
 /* Regular expressions for hexadecimal numbers in ArchC */
 /********************************************************/

<HEXA>{HEXA_DIGIT}+   {  /* Handling Hexadecimal numbers */
     #if DEBUG_LEX
                    printf( "An hexa integer: 0x%s (%d)\n", yytext,(int)strtol( yytext, NULL, 16 ));
     #endif
                    yylval.value = (int)strtol( yytext, NULL, 16 );
                    BEGIN(INITIAL);
		    return INT; 
   } 
 
<HEXA><<EOF>>   {
         fprintf( stderr, "ArchC Lexical ERROR: Unexpected end of file\n" );
         yyterminate();
   }

<HEXA>.   { 
                fprintf( stderr, "ArchC Lexical ERROR: Unrecognized character in hexa state: %s\n", yytext );
                BEGIN(INITIAL);
		yyterminate();
   } 

 /*****************************************************/
 /* Regular expressions for handling Strings in ArchC */
 /*****************************************************/

<STRING>\"   { /* Handling Strings*/
                /* saw closing quote - all done */
                *string_buf_ptr = '\0';
                /* return string constant token type and
                 * value to parser
                 */
     #if DEBUG_LEX
		 printf( "A string: %s\n",string_buf);  
		 printf( "Returning token: %d\n",STR);  
     #endif
                 yylval.text = (char*)malloc(strlen(string_buf)+1);
		 strcpy(yylval.text,string_buf);
		 BEGIN(return_state);
		 return STR;
   }

<STRING>\n   {
               /* error - unterminated string constant */
               /* generate error message */
               *string_buf_ptr = '\0';
               printf( "AC_ERROR: Line %d: Unterminated string constant: %s\n", line_num,string_buf  );
		yyterminate();
   }

<STRING>.   {
               *string_buf_ptr++ = *yytext;
   }

<STRING><<EOF>>   {
         fprintf( stderr, "ArchC Lexical ERROR: Unexpected end of file\n" );
         yyterminate();
   }

 /*****************************************************/
 /* Regular expressions for comments in ArchC */
 /*****************************************************/
<COMMENT>"*/" {
 
#if DEBUG_LEX
	  printf( "End of comment in line: %d\n",line_num);
#endif
		 BEGIN(return_state);
   }

<COMMENT>\n   { 

     line_num++;
     #if DEBUG_LEX
             printf( "Comment. New line: %d\n",line_num);
     #endif
   }

<COMMENT>. {
   }

<COMMENT><<EOF>>   {
	   BEGIN(INITIAL);
	   fprintf( stderr, "ArchC Lexical ERROR: Unexpected end of file\n" );
	   yyterminate();
   }


 /* Comment style regular expressions*/
<COMMENT1>\n   { 

     line_num++;
     #if DEBUG_LEX
             printf( "Leaving comment1. New line: %d\n",line_num);
     #endif
		 BEGIN(return_state);
   }

<COMMENT1>.   {
          
   }

<COMMENT1><<EOF>>   {
	   BEGIN(INITIAL);
	   fprintf( stderr, "ArchC Lexical ERROR: Unexpected end of file\n" );
	   yyterminate();
   }

 /*****************************************************/
 /* Regular expressions for blocks in ArchC */
 /*****************************************************/
<BLOCK_START>"{"|"(" {

  if (*yytext == '{') brace_level++;
  if (*yytext == '(') paren_level++;

  block_buf_ptr = block_buf; 
  BEGIN(BLOCK_STATE);
}

<BLOCK_START>. {
  printf("ArchC Lexical ERROR: Block must begin at line %d\n", line_num);
  yyterminate();
}

<BLOCK_STATE>"{"|"(" {

  if (*yytext == '{') brace_level++;
  if (*yytext == '(') paren_level++;

  *block_buf_ptr++ = *yytext;
}

<BLOCK_STATE>"}"|")" {

  if (*yytext == '}') brace_level--;
  if (*yytext == ')') paren_level--;

  if (! ((brace_level == 0) && (paren_level == 0)))
    *block_buf_ptr++ = *yytext;

  else {
    /* end block state */
    *block_buf_ptr = '\0';
    yylval.text = (char*)malloc(strlen(block_buf)+1);
    strcpy(yylval.text,block_buf);
    BEGIN(return_state);
    return BLOCK;
  }
}

<BLOCK_STATE>\n   { 
  line_num++;
  *block_buf_ptr++ = *yytext;
}

<BLOCK_STATE>. {
  *block_buf_ptr++ = *yytext;
}

<BLOCK_STATE><<EOF>>   {
  BEGIN(INITIAL);
  fprintf( stderr, "ArchC Lexical ERROR: Unexpected end of file inside a block\n" );
  yyterminate();
}

%%

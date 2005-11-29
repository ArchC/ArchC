/* Evaluate C expressions in unsigned type */

%{
#define YYSTYPE unsigned

unsigned eval_result = 0;
char *   eval_input;

void yyerror (const char *s) {}

%}

/* BISON Declarations */
%token   NUM
%token   END
%token   SRL
%token   SLL

/* C Operator Precedence and Associativity       */
/* http://www.difranco.net/cop2220/op-prec.htm   */

/* %left ',' ';' */
/* %right '=' '+=' '-=' '*=' '/=' '%=' '&=' '^=' '|=' '<<=' '>>=' */
/* /\* Ternary conditional *\/ */
/* %left '||' */
/* %left '&&' */
/* %left '|' */
/* %left '^' */
/* %left '&' */
/* %left '==' '!=' */
/* %left '<' '<=' '>' '>=' */
/* %left '<<' '>>' */
/* %left '+' '-' */
/* %left '*' '/' '%' */
/* %right '++' '--' PLUS NEG '!' '~' /\*type cast,deference,address,sizeof*\/ */
/* %left /\*parenthesis,brackets*\/ '.' '->' */


%right '='
%left '|'
%left '^'
%left '&'
%left SRL SLL
%left '-' '+'
%left '*' '/'
%left NEG     /* Negation--unary minus */


%%


string:   exp END            { eval_result = $1; YYACCEPT;      }
;

exp:      NUM                { $$ = $1;                         }
        | exp '+' exp        { $$ = $1 + $3;                    }
        | exp '-' exp        { $$ = $1 - $3;                    }
        | exp '*' exp        { $$ = $1 * $3;                    }
        | exp '/' exp        { $$ = $1 / $3;                    }
        | '-' exp  %prec NEG { $$ = -$2;                        }
        | exp '^' exp        { $$ = $1 ^ $3;                    }
        | exp '|' exp        { $$ = $1 | $3;                    }
        | exp '&' exp        { $$ = $1 & $3;                    }
        | exp SRL exp        { $$ = $1 >> $3;                   }
        | exp SLL exp        { $$ = $1 << $3;                   }
        | '(' exp ')'        { $$ = $2;                         }
;


%%


#include <ctype.h>

int
yylex (void)
{
  /* skip white space  */
  for (; *eval_input == ' ' || *eval_input == '\t'; eval_input++);

  /* process numbers   */
  if (isdigit (*eval_input)) {
    unsigned size;
    yylval = strtoll (eval_input, &eval_input, 0);
/*     sscanf (eval_input, "%u%n", &yylval, &size); */
/*     eval_input += size; */
    return NUM;
  }

  /* return end-of-string */
  if (*eval_input == '\0')
    return END;

  /* return SRL */
  if ((*eval_input == '>') && (*(eval_input+1) == '>')) {
    eval_input += 2;
    return SRL;
  }
  /* return SLL */
  if ((*eval_input == '<') && (*(eval_input+1) == '<')) {
    eval_input += 2;
    return SLL;
  }

  /* return end-of-string  */
  if (*eval_input == '\0')
    return END;


  /* return single chars */
  return *eval_input++;
}

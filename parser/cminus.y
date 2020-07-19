/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static char * savedIdName[10000];
static int savedId = 0;
static char * savedFuncName;
static char * savedCallName[10000];
static int savedCall = 0;
static char * savedNum;
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token IF ELSE WHILE RETURN INT VOID 
%token THEN END REPEAT UNTIL READ WRITE
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN SEMI
%token LBRACE RBRACE LCURLY RCURLY COMMA
%token ERROR 

%nonassoc NO_ELSE
%nonassoc ELSE
%% /* Grammar for CMINUS */

program     : decl_seq
                 { savedTree = $1;} 
            ;
decl_seq	: decl_seq decl
				 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
			| decl	{ $$ = $1;	}
			;
decl		: var_decl	{ $$ = $1;	}
			| fun_decl	{ $$ = $1;	}
			;
var_decl	: INT ID SEMI 
				{ $$ = newStmtNode(VarK);
				  $$->attr.name = copyString(currString);
				  $$->type = Integer;
				}
			| VOID ID SEMI 
				{ $$ = newStmtNode(VarK);
				  $$->attr.name = copyString(currString);
				  $$->type = Void;
				}
			| INT ID { savedName = copyString(currString); }
			LBRACE NUM { savedNum = copyString(tokenString); }
			RBRACE SEMI 
				{ $$ = newStmtNode(VarK);
				  $$->attr.name = copyString(savedName);
				  $$->size = atoi(savedNum);
				  $$->type = Integer;
				}
			| VOID ID { savedName = copyString(currString);}
			LBRACE NUM { savedNum = copyString(tokenString); }
			RBRACE SEMI 
				{ $$ = newStmtNode(VarK);
				  $$->attr.name = copyString(savedName);
				  $$->size = atoi(savedNum);
				  $$->type = Void;
				}
			;
fun_decl	: INT ID { savedFuncName = copyString(currString); } fun_decl2
				{ $$ = $4;
				  $$->attr.name = copyString(savedFuncName);
				  $$->type = Integer;
				}
			| VOID ID { savedFuncName = copyString(currString); } fun_decl2
				{ $$ = $4;
				  $$->attr.name = copyString(savedFuncName);
				  $$->type = Void;
				}
			;
fun_decl2	: LPAREN params RPAREN comp_stmt
				{ $$ = newStmtNode(FunK);
				  $$->child[0] = $2;
				  $$->child[1] = $4;
				}
			;
params		: param_list { $$ = $1; }
			| VOID  { $$ = newStmtNode(ParamK);
					  $$->type = Void;}
			;
param_list	: param_list COMMA param
				 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; 
				   }
                   else $$ = $3;
                 }
			| param { $$ = $1; } 
			;
param		: INT ID
				{ $$ = newStmtNode(ParamK);
				  $$->attr.name = copyString(currString);
				  $$->type = Integer;
				}
			| INT ID { savedName = copyString(currString); }
			  LBRACE RBRACE
				{ $$ = newStmtNode(ParamK);
				  $$->attr.name = copyString(savedName);
				  $$->size = 0;
				  $$->type = Integer;
				}
			| VOID ID
				{ $$ = newStmtNode(ParamK);
				  $$->attr.name = copyString(currString);
				  $$->type = Void;
				}
			| VOID ID { savedName = copyString(currString); }
			  LBRACE RBRACE
				{ $$ = newStmtNode(ParamK);
				  $$->size = 0;
				  $$->attr.name = copyString(savedName);
				  $$->type = Void;
				}
			;
comp_stmt	: LCURLY local_decls stmt_list RCURLY
				{ $$ = newStmtNode(CompK);
					$$->child[0] = $2;
					$$->child[1] = $3;
				}
			;
local_decls	: local_decls var_decl
				{ YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; 
				   }
                   else $$ = $2;
                 }

			| { $$ = NULL; }
			;
stmt_list	: stmt_list stmt
				{ YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; 
				   }
                   else $$ = $2;
                 }
			| { $$ = NULL; } 
			;
stmt		: exp_stmt { $$ = $1; }
			| comp_stmt { $$ = $1; }
			| select_stmt { $$ = $1; }
			| iter_stmt { $$ = $1; }
			| return_stmt { $$ = $1; }
			;
exp_stmt	: exp SEMI { $$ = $1; }
			| SEMI
			;
select_stmt	: IF LPAREN exp RPAREN stmt %prec NO_ELSE
			  { $$ = newStmtNode(SelK1);
				$$->child[0] = $3;
				$$->child[1] = $5;
			  }
			| IF LPAREN exp RPAREN stmt ELSE stmt
				{ $$ = newStmtNode(SelK2);
				  $$->child[0] = $3;
				  $$->child[1] = $5;
				  $$->child[2] = $7;
				}
			;
iter_stmt	: WHILE LPAREN exp RPAREN stmt
			  { $$ = newStmtNode(IterK);
				$$->child[0] = $3;
				$$->child[1] = $5;
			  }
			;
return_stmt	: RETURN SEMI 
			  { $$ = newStmtNode(RetK); }
			| RETURN exp SEMI
				{ $$ = newStmtNode(RetK);
				  $$->child[0] = $2;
				}
			;
exp			: var ASSIGN exp
			  { $$ = newStmtNode(AssignK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
			  }
			| simple_exp { $$ = $1; }
			;
var			: ID 
			  { $$ = newExpNode(IdK);
				   $$->attr.name = copyString(currString);
			  }
			| ID { savedIdName[savedId++] = copyString(currString); }
			  LBRACE exp RBRACE
				{ $$ = newExpNode(IdK2);
					$$->attr.name = copyString(savedIdName[--savedId]);
					$$->child[0] = $4;
				}
			;
simple_exp	: add_exp relop add_exp
				{ $$ = $2;
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				}
			| add_exp { $$ = $1; }
			;
relop		: LT { $$ = newExpNode(OpK); $$->attr.op=LT;}
			| LE { $$ = newExpNode(OpK); $$->attr.op=LE;}
			| GT { $$ = newExpNode(OpK); $$->attr.op=GT;}
			| GE { $$ = newExpNode(OpK); $$->attr.op=GE;}
			| EQ { $$ = newExpNode(OpK); $$->attr.op=EQ;}
			| NE { $$ = newExpNode(OpK); $$->attr.op=NE;}
			;
add_exp		: add_exp PLUS term
				{ $$ = newExpNode(OpK);
				  $$->attr.op = PLUS;
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				}
			| add_exp MINUS term
				{ $$ = newExpNode(OpK);
				  $$->attr.op = MINUS;
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				}
			| term { $$ = $1; }
			;
term		: term TIMES factor
				{ $$ = newExpNode(OpK);
				  $$->attr.op = TIMES;
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				}
			| term OVER factor
				{ $$ = newExpNode(OpK);
				  $$->attr.op = OVER;
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				}
			| factor { $$ = $1; }
			;
factor		: LPAREN exp RPAREN { $$ = $2; }
			| var{ $$ = $1; }
			| call{ $$ = $1; }
			| NUM
				{ $$ = newExpNode(ConstK);
					$$->attr.val = atoi(tokenString);
				}
			;
call		: ID { savedCallName[savedCall++] = copyString(currString);}
			  LPAREN args RPAREN
				{ $$ = newStmtNode(CallK); 
				 $$->attr.name = copyString(savedCallName[--savedCall]);
				 $$->child[0] = $4;
				}
			;
args		: arg_list{ $$ = $1; }
			| { $$ = NULL; }
			;
arg_list	: arg_list COMMA exp
			  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; 
				   }
                   else $$ = $3;
                 }
			| exp{ $$ = $1; }
			;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}


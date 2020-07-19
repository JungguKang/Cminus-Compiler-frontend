/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#define SIZE 211
/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name,ExpType type, int lineno );
typedef struct LineListRec
{
	int lineno;
	struct LineListRec * next;
} * LineList;
typedef struct BucketListRec
{ char * name;
    char * type;
	LineList lines;
	int memloc;
	struct BucketListRec * next;
} * BucketList;

typedef struct ScopeListRec
{
	char * name;
	BucketList bucket[SIZE];
	struct ScopeListRec * parent;
	int location;
	int compNum;
} * ScopeList;

typedef struct FuncListRec
{
	char * name;
	char * type;
	int paramNum;
	BucketList param[SIZE];
} * FuncList;


//static int isFunc = 0;
//static int scope_added=0;
void scope_goParent();
void scope_insert();
void scope_saveName(char *name);
void makeScope();
void printCurrScope();
void build_funcTable(char * name, ExpType type);
void insert_param(char * param, ExpType type);
FuncList search_func(char * name);
ScopeList search_scope();
BucketList search_var(char * name, ScopeList sc);
BucketList search_var_inscope(char *name, ScopeList sc);
void scope_enter();

int retIsFunc();
int retSA();
void sa_one();
void sa_zero();
void isfunc_zero();
void isfunc_one();
static FuncList funcTable[SIZE];
static int funcNum = 0;
/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
/*int st_lookup ( char * name );*/
int st_lookup (char * name);
int st_lookup_withParent(char * name);

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif

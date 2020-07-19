/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"


/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4


static char * savedName;
static int scopeNum=0;
extern char * copyString(char * s);
static ScopeList temp_scope;
static int isFunc = 0;
static int scope_added = 0;

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

int retIsFunc()
{
	return isFunc;
}
int retSA()
{
	return scope_added;
}

void sa_zero()
{
	scope_added = 0;
}

void sa_one()
{
	scope_added = 1;
}
void isfunc_zero()
{
	isFunc = 0;
}
void isfunc_one()
{
	isFunc = 1;
}


/* the scope table (test)*/
static ScopeList curr_scope;
static ScopeList scopeTable[SIZE];
/*
void printCurrScope()
{
	fprintf(listing,"current : %s\n",curr_scope->name);
}*/
void makeScope()
{
	ScopeList scopeEntry = (ScopeList) malloc(sizeof(struct ScopeListRec));
	scopeEntry->name = "global";
	scopeEntry->parent = NULL;
	scopeEntry->location = 0;
	scopeEntry->compNum = 0;

	curr_scope = scopeEntry;
	scopeTable[scopeNum++] = curr_scope;
}

/**
  * copy struct ScopeList
  * and return copied ScopeList
  */
ScopeList copyScope(ScopeList scope)
{
	int i;
	ScopeList temp = (ScopeList) malloc(sizeof(struct ScopeListRec));
	temp->name = copyString(scope->name);
	for( i=0; i<SIZE;i++)
		temp->bucket[i] = scope->bucket[i];
	temp->parent = scope->parent;
	temp->location = scope->location;
	temp->compNum = scope->compNum;
	return temp;

}
void scope_insert()
{
	ScopeList newScope = (ScopeList) malloc(sizeof(struct ScopeListRec));
	if(isFunc)
	{
		newScope->name = copyString(savedName); 
		
	}
	else
	{
		sprintf(savedName, "%s.%d",curr_scope->name,curr_scope->compNum);
		newScope->name = copyString(savedName);
		curr_scope->compNum++;
	}
	newScope->parent = curr_scope;
	newScope->location = 0;
	newScope->compNum = 0;
	curr_scope = newScope;
	scopeTable[scopeNum++] = curr_scope;
	
}
void scope_enter()
{
	ScopeList newScope = (ScopeList) malloc(sizeof(struct ScopeListRec));
	if(isFunc)
	{
		newScope->name = copyString(savedName);
	}
	else
	{
		
		sprintf(savedName, "%s.%d",curr_scope->name,curr_scope->compNum);
		newScope->name = copyString(savedName);
		curr_scope->compNum++;
	}
	newScope->parent = curr_scope;
	newScope->location = 0;
	newScope->compNum = 0;
	curr_scope = newScope;
	
}
/**
  * search ScopeList in scopeTable
  * with curr_scope->name
  * if there is a match return
  * else return NULL
  */
ScopeList search_scope()
{
	int i;
	ScopeList a;
	for(i=0;i<scopeNum;i++)
	{
		if(!strcmp(scopeTable[i]->name,curr_scope->name))
		{
			a = copyScope(scopeTable[i]);
				return scopeTable[i];
				}
	}
	return NULL;
}
/** search for variable in scopetable
  * input : variable name, starting scope
  * return NULL if there is no existing variable
  * return BucketList if there is matching variable
  */
BucketList search_var(char * name, ScopeList sc)
{
	int i;
	int h = hash(name);
	BucketList l;
	while(sc != NULL)
	{
		l=sc->bucket[h];
		while ((l != NULL) && (strcmp(name,l->name) != 0))
    		l = l->next;
		if((l != NULL) && (!strcmp(l->name,name)))
		{
			return l;
		}
		if(sc->parent == NULL)
		{
			sc = NULL;
		}
		else
			sc = sc->parent;
	}
	return NULL;
}
BucketList search_var_inscope(char * name, ScopeList sc)
{
	int i;
	int h = hash(name);
	BucketList l;
	if(sc != NULL)
	{
		l=sc->bucket[h];
		while ((l != NULL) && (strcmp(name,l->name) != 0))
    		l = l->next;
		if((l != NULL) && (!strcmp(l->name,name)))
		{
			return l;
		}
	
	}
	return NULL;
}
void printScope()
{
	if(curr_scope==NULL)
		fprintf(listing,"no currscope\n");
	else
		fprintf(listing,"%s\n",curr_scope->name);
}



void scope_saveName(char * name)
{
	savedName = copyString(name);
	isFunc = 1;
}
void scope_goParent()
{

	curr_scope = curr_scope->parent;
}


void build_funcTable(char * name,ExpType type )
{
	FuncList newFunc = (FuncList) malloc(sizeof(struct FuncListRec));
	newFunc->name = name;
	if(type == 0)
		newFunc->type = "Void";
	else
		newFunc->type = "Integer";
	newFunc->paramNum = 0;
	funcTable[funcNum++] = newFunc;
	
}

FuncList search_func(char * name)
{
	int i;
	for(i=0;i<funcNum;i++)
	{
		if(!strcmp(funcTable[i]->name,name))
			return funcTable[i];
	}
	return NULL;
}
void insert_param(char * param, ExpType type)
{
	BucketList p = (BucketList) malloc(sizeof(struct BucketListRec));
	p->name = param;
	if(type == 1)
		p->type = "Integer";
	else if(type == 2)
		p->type = "Integer Array";
	p->memloc = 0;
	funcTable[funcNum-1]->param[funcTable[funcNum-1]->paramNum] = p;
	funcTable[funcNum-1]->paramNum++;

}
	

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name,ExpType type ,int lineno )
{ int h = hash(name);
  BucketList l =  curr_scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { curr_scope->location++;
	  l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = copyString(name);
	if(type == 0)
		l->type = "Void";
	else if(type == 1)
		l->type = "Integer";
	else if(type == 2)
		l->type = "Integer Array";
	else return;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;

    l->memloc = curr_scope->location;
    l->lines->next = NULL;
    l->next = curr_scope->bucket[h];
    curr_scope->bucket[h] = l;
  }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL
			) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
	curr_scope = copyScope(temp_scope);
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{ int h = hash(name);
  BucketList l = curr_scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

int st_lookup_withParent( char * name)
{
	int h = hash(name);
	ScopeList temp = copyScope(curr_scope);
	while(temp != NULL)
	{
		BucketList l= temp->bucket[h];
		while((l != NULL) && (strcmp(name, l->name) != 0)){
			l=l->next;
		}

		if(l == NULL)
		{
			if(temp->parent)
				temp = copyScope(temp->parent);
			else
				return -1;
		}
		else
		{

			temp_scope = copyScope(curr_scope);
			curr_scope = temp;
			if(!strcmp(l->type,"Integer"))
				return 1;
			else if(!strcmp(l->type,"Integer Array"))
				return 2;
			return l-> memloc;

		}
	}
	return -1;

}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i,j;
  fprintf(listing,"Variable Name  Variable Type  Scope Name  Location  Line Numbers\n");
  fprintf(listing,"-------------  -------------  ----------  --------  ------------\n");
  for(j=0; j<scopeNum ; j++){
	  curr_scope = scopeTable[j];
	  //fprintf(listing,"%d scope\n",j);
  for (i=0;i<SIZE;++i)
  { 
	  if (curr_scope->bucket[i] != NULL)
    	{ 	
			BucketList l = curr_scope->bucket[i];
      		while (l != NULL)
      		{ 	LineList t = l->lines;
				//ScopeList s = l->scope;
        		fprintf(listing,"%-13s  ",l->name);
				fprintf(listing,"%-13s  ",l->type);
				fprintf(listing,"%-10s  ",curr_scope->name);//find scope name
        		fprintf(listing,"%-8d  ",l->memloc);
        		while (t != NULL)
        		{ fprintf(listing,"%4d ",t->lineno);
        	  		t = t->next;
        		}
	//			fprintf(listing,"%10d ",i);
        		fprintf(listing,"\n");
        		l = l->next;
      		}	
     	}
		
  	}
  //curr_scope = curr_scope->parent;
  }

  fprintf(listing,"Function Name  Return Name  Parameter name  Parameter type\n");
  fprintf(listing,"-------------  -----------  --------------  --------------\n");
  for(j=0;j<funcNum;j++)
  {
	  FuncList k = funcTable[j];
	  fprintf(listing,"%-13s  ",k->name);
	  fprintf(listing,"%-11s  ",k->type);

	  for(i=0;i<k->paramNum;i++)
	  { 
		  if(i != 0)
			  fprintf(listing,"                            ");
	  	fprintf(listing,"%-14s  ",k->param[i]->name);
	  	fprintf(listing,"%-14s  ",k->param[i]->type);
		fprintf(listing,"\n");
	  }
	  fprintf(listing,"\n");
  }

} /* printSymTab */

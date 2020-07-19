/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
//static int location[211];


/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
int dbg = 0;
static int func_type;

static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
	  {
        traverse(t->child[i],preProc,postProc);
	  }
    }
	postProc(t);
    traverse(t->sibling,preProc,postProc);
	//postProc(t);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static void escapeComp(TreeNode *t)
{
	if(t->nodekind == StmtK)
	{
		if(t->kind.stmt == CompK){
		scope_goParent();
		return;
		}
	}
	return;

}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { 
		  case FunK:
			  
			  scope_saveName(t->attr.name);
			  if(st_lookup(t->attr.name) == -1){
				  st_insert(t->attr.name,t->type, t->lineno);
				  build_funcTable(t->attr.name, t->type);
			  } 
			  //need error
			  else
				  break;
			  break;
		  case VarK:
			  if(st_lookup(t->attr.name) == -1){
				  if(t->size==1)
				  	st_insert(t->attr.name,t->type, t->lineno);
				  else
					  st_insert(t->attr.name,2,t->lineno);
			  }
			  break;
		  case ParamK:
			  if(!retSA()){
				  scope_insert();sa_one();
			  }
			  if(t->attr.name == NULL)
				  break;
			 if(st_lookup(t->attr.name) == -1){
				 if(t->size == 1){
				  st_insert(t->attr.name,1, t->lineno);
				  insert_param(t->attr.name,1);
				 }
				 else if(t->size == 0){
					 st_insert(t->attr.name,2,t->lineno);
					 insert_param(t->attr.name, 2);
				 }
				 else
					 break;//error
			  }
			  break;
		  case CompK:
			  if(retIsFunc())
			  {
				  sa_zero();
				  isfunc_zero();
				  break;
			  }
			  else
		  	  	scope_insert();
			  break;

		case CallK:
			  if(st_lookup_withParent(t->attr.name) == -1){
				  break;
			  }
			  else{
				  st_insert(t->attr.name,t->type,t->lineno);
			  }
			  break;

        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
          if (st_lookup_withParent(t->attr.name) == -1){}
          else
            st_insert(t->attr.name,t->type, t->lineno);

          break;
		case IdK2:
		   if (st_lookup_withParent(t->attr.name) == -1){}
          else
            st_insert(t->attr.name,t->type, t->lineno);
		  break;
		default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ makeScope();
	st_insert("input",1,0);
	st_insert("output",0,0);
	build_funcTable("input",1);
	build_funcTable("output",0);
	insert_param("",1);
	traverse(syntaxTree,insertNode,escapeComp);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}
static char * checkScope; 
static FuncList called_func;
/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { 
        case ConstK:
        case IdK:
          //t->type = Integer;
		  //check if it is defined(is checked when table is made) ->no need to imple
		  //check if this is right type single/array type (table made?)->no need to imple
          break;
		case IdK2:
		  break;
		case OpK:
		  //check if child on bothhand is int
		  //can be OpK->skip since next opk will do the job
		 if(t->child[0] != NULL)
		  {
		  
			  BucketList newBuc;

			  if(t->child[0]->nodekind == StmtK)
			  {//when callk
				  if(t->child[0]->kind.stmt == CallK)
				  {
					  called_func = search_func(t->child[0]->attr.name);
				  	  t->child[0]->type = 1;
					  if(called_func == NULL){
						  typeError(t->child[0],"no such function declared");
						  t->child[0]->type = 0;
					  }
					  else if(strcmp(called_func->type,"Integer")){
							  typeError(t->child[0],"function type error");
						  t->child[0]->type = 0;
					  }
				  }
			  }
			  else if(t->child[0]->nodekind == ExpK)
			  {//when constk, idk, idk2
				  if(t->child[0]->kind.exp == ConstK)
					  t->child[0]->type = 1;
				  else if(t->child[0]->kind.exp == IdK)
				  {
					  newBuc = search_var(t->child[0]->attr.name,search_scope());
				  	  t->child[0]->type = 1;
					  if(newBuc==NULL)
					  {
						  typeError(t->child[0],"no such variable declared");
						  t->child[0]->type = 0;
					  }
					  else if(strcmp(newBuc->type,"Integer"))
					  {
						  typeError(t->child[0],"variable not int");
						  t->child[0]->type = 0;
					  }
				  }
				  else if(t->child[0]->kind.exp == IdK2)
				  {
					  newBuc = search_var(t->child[0]->attr.name,search_scope());
				  	  t->child[0]->type = 1;
					  if(newBuc==NULL)
					  {
						  typeError(t->child[0],"no such variable declared");
						  t->child[1]->type = 0;
					  }
					  else if(strcmp(newBuc->type,"Integer Array")){
						  typeError(t->child[0],"variable not int");
						  t->child[1]->type = 0;
					  }
				  }
			  }
			  if(t->child[1]->nodekind == StmtK)
			  {//when callk
				  if(t->child[1]->kind.stmt == CallK)
				  {
					  called_func = search_func(t->child[1]->attr.name);
				  	  t->child[1]->type = 1;
					  if(called_func == NULL){
						  typeError(t->child[1],"no such function declared");
						  t->child[1]->type = 0;
					  }
					  else if(strcmp(called_func->type,"Integer")){
							  typeError(t->child[1],"call type error");	
						  t->child[1]->type = 0;
					  }
				  
				  }
			  }
			  else if(t->child[1]->nodekind == ExpK)
			  {//when constk, idk, idk2

				  if(t->child[1]->kind.exp == ConstK)
					  t->child[1]->type = 1;
				  else if(t->child[1]->kind.exp == IdK)
				  {
					  newBuc = search_var(t->child[1]->attr.name,search_scope());
				  	  t->child[1]->type = 1;
					  if(newBuc==NULL){
						  typeError(t->child[1],"no such variable declared");
						  t->child[1]->type = 0;
					  }
					  else if(strcmp(newBuc->type,"Integer")){
						  typeError(t->child[1],"variable not int");
						  t->child[1]->type = 0;
					  }
				  }
				  else if(t->child[1]->kind.exp == IdK2)
				  {
					  newBuc = search_var(t->child[1]->attr.name,search_scope());
				  	  t->child[1]->type = 1;
					  if(newBuc==NULL){
						  typeError(t->child[1],"no such variable declared");
						  t->child[1]->type = 0;
					  }
					  else if(strcmp(newBuc->type,"Integer Array")){
						  typeError(t->child[1],"variable not int");
						  t->child[1]->type = 0;
					  }
				  }

			  }
			//  if((t->child[0]->type != 1) || (t->child[1]->type != 1))
			//	  typeError(t,"Op applied to non-integer");
			  if((t->attr.op == EQ) || (t->attr.op == LT) ||
					  (t->attr.op==GT)||(t->attr.op == GE)||
					  (t->attr.op==LE)||(t->attr.op==NE))
				  t->type = 3;
			  else
				  t->type = 1;

		  }
		  break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { 
		  case FunK:
			  checkScope = t->attr.name;
			  break;
		  case VarK:
			  if(t->type == Void)
				  typeError(t,"void type variable not allowed");
			  break;
		  case AssignK:
			  if(t->child[0] != NULL)
			  {
				  BucketList buc = search_var(t->child[0]->attr.name,search_scope());
				  if(buc == NULL){
					  typeError(t->child[0],"undifined variable");
					  break;
				  }
				  if(t->child[0]->kind.exp==IdK)
				  {
					  if(strcmp(buc->type,"Integer"))
							  typeError(t->child[0],"in assign: defined different type");
				  }
				  else if(t->child[0]->kind.exp==IdK2)
				  {
					  if(strcmp(buc->type,"Integer Array"))
						  typeError(t->child[0],"in assign: defined different type");
				  }
				  else break;
			  }
			  if(t->child[1] !=NULL)
			  {//call, opk, const, idk idk2 should be integer type
				  if(t->child[1]->nodekind == StmtK)
				  {
					  if(t->child[1]->kind.stmt == CallK)
					  {
						  called_func = search_func(t->child[1]->attr.name);
						  if(called_func == NULL)
						  {
							  typeError(t->child[1],"no function defined");
							  break;
						  }
						  if(strcmp(called_func->type,"Integer"))
							  typeError(t->child[1],"in assign: called function not integer");
					  }
				  }
				  else if(t->child[1]->nodekind == ExpK)
				  {
					  if(t->child[1]->kind.exp == ConstK || t->child[1]->kind.exp == OpK)
						  break;
					   BucketList buc = search_var(t->child[1]->attr.name,search_scope());
				  	if(buc == NULL){
					  	typeError(t->child[1],"undifined variable");
						break;
					}
				  	if(t->child[1]->kind.exp==IdK)
				  	{
					  	if(strcmp(buc->type,"Integer"))
							  typeError(t->child[1],"in assign: defined different type");
				  	}
				  	else if(t->child[1]->kind.exp==IdK2)
				  	{
					  	if(strcmp(buc->type,"Integer Array"))
						  	typeError(t->child[1],"in assign: defined different type");
				  	}	
					else if(t->child[1]->kind.exp == OpK)
					{
						if(t->child[1]->type == 3)
							typeError(t->child[1],"right hand side, relation operation not permitted");
					}
				  }
			  }


			  break;
		  case RetK:
			  if(t->child[0] == NULL)
			  {
				  if(func_type == 0)
					  break; 
				  else
					  typeError(t,"Wrong return type");
				  break;
			  }
				
			  else if(t->child[0]->nodekind == ExpK)
			  {
				  if(t->child[0]->kind.exp == ConstK)
				  {
					  if(func_type == 1)
						  break;
					  else
						  typeError(t->child[0],"Wrong return type");
				  }
				  if(t->child[0]->kind.exp == IdK)
				  {
					  if(func_type == 1)
					  {
						  BucketList newBucket = search_var(t->child[0]->attr.name,search_scope());
						  if(newBucket == NULL)
							  typeError(t->child[0],"No such variable declared");
						  else if(strcmp(newBucket->type,"Integer"))
							  typeError(t->child[0],"Wrong return type");
					  }
					  else
						  typeError(t->child[0],"Wrong return type");
				  }
				  if(t->child[0]->kind.exp == IdK2)
				  {//if func return type int ok else no
				   // 

					  if(!(func_type == 1))
						  typeError(t->child[0],"Wrong return type");


				  }
			  }
			  else if(t->child[0]->nodekind == StmtK)
			  {
				  if(t->child[0]->kind.stmt == CallK)
				  {//check called function type functype should matched with called
					  called_func=search_func(t->child[0]->attr.name);
					  if(called_func == NULL)
					  {
						  typeError(t,"no such function declared");
						  break;
					  }
					  if(func_type==1 && strcmp(called_func->type,"Integer") )
						  typeError(t,"wrong return type");
					  else if(func_type==0 && strcmp(called_func->type,"Void"))
						  typeError(t,"wrong return type");
				  }
				  else if(t->child[0]->kind.stmt == AssignK)
				  {//check assigning variable type func should be integer
					  if(!(func_type == 1))
					  {
						  typeError(t,"wrong return type");
					  }
				  }
			  }
			  break;
		  case CallK:
			  if(called_func = search_func(t->attr.name))
			  {
				  TreeNode * tempT;
				  int i=0;
				  BucketList cmpSub;
				  tempT = t->child[0];

				  while(tempT)
				  {
					  //fprintf(listing,"2entered call not expk %s, %s\n",t->attr.name,tempT->attr.name);

				 	  if(called_func->param[i] == NULL)
					  {
						  typeError(t,"too many arguenemtns");
						  break;
					  }
					  if(tempT->nodekind == ExpK && tempT->kind.exp == IdK)
					  {
						 cmpSub= search_var(tempT->attr.name,search_scope());

						 if(cmpSub == NULL)
						 {
							 typeError(t,"No such id declared");
						 }
					//	 if(strcmp(cmpSub->type,"Integer"))
					//		 typeError(t,"arguement type error");

				 	 	if(cmpSub!=NULL && strcmp(cmpSub->type,called_func->param[i]->type))
							  typeError(tempT,"Wrong arguement type");
				 	// 	else if(cmpSub!=NULL && cmpSub->type == 0)
					//		  typeError(tempT,"Wrong arguement type");
					  }
					  else if(tempT->nodekind == ExpK && tempT->kind.exp == IdK2)
					  {
						  cmpSub= search_var(tempT->attr.name,search_scope());

						 if(cmpSub == NULL)
						 {
							 typeError(t,"No such id declared");
						 }
						 if(strcmp(cmpSub->type,"Integer Array"))
							 typeError(t,"arguement type error");

				 	 	if(cmpSub!=NULL && strcmp("Integer",called_func->param[i]->type))
							  typeError(tempT,"Wrong arguement type");
				 	 	

					  }
					  else if(tempT->nodekind == ExpK && tempT->kind.exp == ConstK)
					  { 
						 
						  if(strcmp("Integer",called_func->param[i]->type))
						  	typeError(tempT,"Wrong arguement type: integer inserted");
						 
					  }
					  else if(tempT->nodekind == ExpK && tempT->kind.exp==OpK)
					  {//check result type
						  //case OpK will execute when operands are not adequate

						  //check if param is integer fit
						  if(strcmp(called_func->param[i]->type,"Integer"))
							  typeError(tempT,"Wrong arguement type: integer inserted");
						  
					  }
					  tempT = tempT->sibling;
					  i++;
		
			  		}
			  }

			  break;
		  case CompK:
			  scope_goParent();
			  break;


		  default:
         	 break;
      }
      break;
    default:
      break;

  }
}

void checkFunc(TreeNode *t)
{
	if(t->nodekind == StmtK)
	{
		if(t->kind.stmt == FunK)
		{
			BucketList newbuc = search_var(t->attr.name,search_scope());
			if(newbuc->lines->lineno != t->lineno)
				typeError(t,"function redefined");
		
				func_type = t->type;
				scope_saveName(t->attr.name);
				scope_enter();
			

		}
		if(t->kind.stmt == VarK)
		{
			BucketList newbuc = search_var_inscope(t->attr.name,search_scope());
			if(newbuc != NULL && newbuc->lines->lineno != t->lineno)
			{
				typeError(t,"Variable name used again");
			}
		}
		if(t->kind.stmt == CompK)
		{
			if(retIsFunc())
				isfunc_zero();
			else
				scope_enter();
		}
				

	}
	
}




/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,checkFunc,checkNode);
}

%{ 
/*
Name: Meng-Yang Tseng 
Date: April 10 2023
lab: lab9

Purpose: step by step to build a CMINUS compiler based on Lex and Yacc, the 1st step is to implement Lex 
         rules in lex file and return tokens or yytext depends on situation, and then build 
         Yacc rules in Yacc file.

         * CFG rules are pdf file, it doesn't have an url, but I have put the text inside of comment below.     
 
I finished all below: 
 * In any other file that needs these variables, use the "extern" directive which says I want to use it, but someone else declared

extern int mydebug;
extern int linecount;


 * Update yyerror() such that it prints the error along with the global lineno;
 * All tokens coming from LEX shall be single characters or upper case and start with "T_" .  For example reserved word "return" shall be returned as T_RETURN.
 * Everywhere you have T_ID in your YACC file, you must print out the name of the variable in YACC  (not in LEX)
 * Non Terminals SHALL be Camel Case :   like Function_Declaration


 change: 
    * added #include "ast.h"
    * added "ASTnode * node; enum AST_MY_DATA_TYPE input_type; enum AST_OPERATORS operator;" into union
    * added T_STRING into token <string> (originally belonged to token)
    * changed uminus rule from '-' Factor to  T_MINUS Factor (lex can't recognize the difference between minus and uminus, but yacc can!)
    * added %type <node>, <input>, <operator> for creating node
    * added printf("\nFinished Parsing\n\n\n"); ASTprint(0, PROGRAM); into int main
    * and other staffs which have comment like added on lab6

 change on lab 7:
    * modified Function_Declaration for prototype and normal function 
    * added CreateTemp() for every expression and arg
    * added LEVEL, OFFSET, GOFFSET, maxoffset global variable 
    * added Display(); and printf("\n\nAST PRINT\n\n"); into main()
    * added type checking for every expression
  
  change on lab 9:
    * added commend line
    * added grammar rule for break and continue
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> /* added for exit */
#include <string.h> /* added on lab 9*/
#include "ast.h" /* added on lab6 */
#include "symtable.h" /* added on lab7 */
#include "emit.h" /* added on lab 9 */

ASTnode * PROGRAM; /* added on lab6 */

int yylex();
extern int lineno; /* var from lex file */
extern int mydebug; /* var from lex file */
int LEVEL = 0; //gloabl context variable to know how deep we are
int OFFSET = 0; //gloabl variable for accumulation needed run-time space
int GOFFSET = 0; //gloabl variable FOR accumulation variable offset 
int maxoffset = 0; // the largest offset needed of the current function

void yyerror(s)
    char *s;
{
  printf ("Yacc parse error: %s on the line %d\n", s, lineno);
} /* for error handling when it happens, it will figure out exact line*/

%} /* start of specs

%start Program /* tell computer how to start the rule, from Program */

%union
{
	int value;
	char* string;
  ASTnode * node; /* added on lab6 */
  enum AST_MY_DATA_TYPE input_type; /* added on lab6 */
  enum AST_OPERATORS operator; /* added on Mar 6 */
}
/* for type of tokens */

%token <string> T_ID T_STRING
%token <value> T_NUM
%token T_INT
%token T_VOID
%token T_IF
%token T_ELSE
%token T_WHILE
%token T_RETURN
%token T_READ
%token T_WRITE
%token T_LE T_L T_B T_BE T_E T_NE
%token T_PLUS T_MINUS 
%token T_MUTIPLE T_DIVIDE T_MOD T_UMINUS
/* above tokens return from lex */
%token T_CONTINUE T_BREAK /* added on lab 9 */

%type <node> Declaration Declaration_list Var_Declaration Var_List Fun_Declaration /* added on lab6 */
%type <node> Params Compound_Stmt Local_Declaration Statement_List Statement Write_Stmt 
%type <node> Expression Simple_Expression Addictive_Expression Term Factor/* added on Mar 6*/
%type <node> Params_List Param Selection_Stmt /* added on Mar 10 */
%type <node> Read_Stmt Var Expression_Stmt Call Args Arg_List Iteration_Stmt Assignment_Stmt/* added on Mar 11 */
%type <node> Return_Stmt /* added on Mar 11 */
%type <node> Fun_End /* added on lab 7 to fix prototype and function */
%type <node> Break_Stmt /* added on lab 9 */
%type <input_type> Type_Specifier 
%type <operator> Addop Relop Multop /* added Relop and Multop on Mar 11 */



%% /* end specs, begin rules */

Program             : Declaration_list  
                      {
                        PROGRAM = $1;
                      } /* added on lab6 */
                    ; /* 1. program → declaration-list */

Declaration_list    : Declaration 
                      {
                        $$ = $1;
                      } /* added on lab6 */
                    | Declaration Declaration_list
                     {
                        $$ = $1;
                        $$ -> next = $2; /* Declaration_list will be one or more*/
                     } 
                    ; /* 2. declaration-list → declaration { declaration } */
 
Declaration         : Var_Declaration 
                     {
                      $$ = $1;
                     } /* added on lab6 */
                    | Fun_Declaration 
                     {
                      $$ = $1;
                     } /* added on lab6, need to take care about this case */
                    ; /* 3. declaration →  var-declaration |  fun-declaration */
 

 
Var_Declaration     : Type_Specifier Var_List ';'
                      {
                        ASTnode *p;
                        p = $2;
                        while (p != NULL)
                        {
                          p -> my_data_type = $1;
                          /* moved p = p-> s1 to the end */
                          if(Search(p->name, LEVEL, 0) != NULL)
                          {
                            yyerror(p->name);
                            yyerror("Symbol already defined");
                            exit(1);
                          } /* added on lab7, to check it is in the symbol table */
                          /* if it is not in the symbol table */
                          if(p->value == 0)
                          { /* a scalar */
                            p->symbol = Insert(p->name, p->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                            OFFSET = OFFSET + 1;
                          }
                          else
                          { /* an array */
                            p->symbol = Insert(p->name, p->my_data_type, SYM_ARRAY, LEVEL, p->value, OFFSET); /* assigned to p->symbol on Mar 25*/
                            OFFSET = OFFSET + p->value;
                          }
                          p = p -> s1;
                        }
                        
                        $$ = $2;
                      } /* added on lab6 */
                    ; /* 4. var-declaration → type-specifier   var-list ; */
 

/* ASTnode *ASTCreateNode(enum ASTtype mytype) */ 
Var_List            : T_ID  
                      {
                        $$ = ASTCreateNode(A_VARDEC);  
                        $$ -> name = $1;
                      } /* added on lab6 */
                    | T_ID '[' T_NUM ']' 
                      {
                        $$ = ASTCreateNode(A_VARDEC);
                        $$ -> name = $1;
                        $$ -> value = $3; 
                      } /* added on Mar 11 */
                    | T_ID ',' Var_List  
                      {
                        $$ = ASTCreateNode(A_VARDEC);  
                        $$ -> name = $1;
                        $$ -> s1 = $3;
                      } /* added on lab6, modified $$ -> s1 to $$ -> next */
                    | T_ID '[' T_NUM ']' ',' Var_List
                      {
                        $$ = ASTCreateNode(A_VARDEC);
                        $$ -> name = $1;
                        $$ -> value = $3;
                        $$ -> s1 = $6;
                      } /* added on Mar 11*/
                    ; /* 5. var-list →    ID [ [ NUM ] ]+ { , ID [ [ NUM ] ]+  } */
                      /* it should use next, but I am not sure how */
                      /* Mar 12 updated, prof said it is ok to use s1 */
                      /* Created A_VARDEC node */

                         

Type_Specifier      : T_INT { $$ = A_INTTYPE;} /* added on lab6 */
                    | T_VOID { $$ = A_VOIDTYPE; } /* added on lab6 */ 
                    ; /* 6. type-specifier →  int | void */
                    /* assigned it to c user define data type */ 

Fun_Declaration     : Type_Specifier T_ID
                      { /* to check T_ID has been used or not */
                        if(Search($2,LEVEL,0) != NULL)
                        { /* ID has been used */
                          yyerror($2);
                          yyerror("function name has been used");
                          exit(1);
                        }
                        /* not in symbol, insert it */
                        Insert($2, $1, SYM_FUNCTION, LEVEL, 0, 0);

                        GOFFSET = OFFSET;
                        OFFSET = 2; /* changed to 2 on lab 9 */
                        maxoffset = OFFSET;
                        /* added on lab7 */
                        
                      } 
                      '(' Params ')' 
                      {
                        Search($2, LEVEL, 0)->fparms = $5; /* added on lab 7 */
                      }
                      Fun_End
                      {
                        $$ = ASTCreateNode(A_FUNCTIONDEC);
                        $$ -> name = $2;
                        $$ -> my_data_type = $1;
                        $$ -> s1 = $5; /* changed from 4 to 5*/
                        $$ -> s2 = $8; /* changed from 6 to 7*/
                        if ($$ -> s2 == NULL)
                        {
                          $$ -> value = 0;
                        }
                        else
                        {
                          $$ -> value = 1;
                        } /* assigned value for normal function and function prototype, o means prototype, 1 means normal function */
                        $$ -> symbol = Search($2, LEVEL, 0); /* added on lab 7*/
                        $$ -> symbol -> offset = maxoffset; /* saved maxoffset to the pointer */
                        OFFSET = GOFFSET; /* added on lab7, reset the offset for gloabl variable*/
                        
                      }
                    ;
                    /* use left factor to deal with function and function prototype */

Fun_End             : Compound_Stmt {$$ = $1;}
                    | ';'{$$ = NULL;}
                    ; /* added Fun_End for prototype and normal function */


Params              : T_VOID { $$ = NULL;} /* added on Mar 10, void means nothing so assign NULL */
                    | Params_List { $$ = $1;} /* added om Mar 10 */
                    ; /* 8. params →  void | param-list */
                    /* void means nothing, so it is NULL */
 
Params_List         : Param 
                     {
                      $$ = $1;
                     } /* added on Mar 10 */
                    | Param ',' Params_List
                      {
                        $$ = $1;
                        $$ -> next = $3;
                      } /* added on Mar 10 */
                    ; /* 9. param-list → param { , param } */
                    /* use next to connect */

Param               : Type_Specifier T_ID 
                     {  
                        if(Search($2, LEVEL+1, 0) != NULL)
                        { /* check the param has been used or not */
                          yyerror($2);
                          yyerror("Parameter already used");
                          exit(1);
                        }
                        $$ = ASTCreateNode(A_PARAMDEC);
                        $$ -> name = $2;
                        $$ -> my_data_type = $1;
                        $$ -> value = 0;

                        /* install the symbol */
                        $$ -> symbol = Insert($$->name, $$->my_data_type, SYM_SCALAR, LEVEL+1, 1, OFFSET);
                        OFFSET = OFFSET + 1;
                     } /* added on Mar 10 */
                    | Type_Specifier T_ID '[' ']' 
                     {  
                        if(Search($2, LEVEL+1, 0) != NULL)
                        { /* check the param has been used or not */
                          yyerror($2);
                          yyerror("Parameter already used");
                          exit(1);
                        } 
                        $$ = ASTCreateNode(A_PARAMDEC);
                        $$ -> name = $2;
                        $$ -> my_data_type = $1;
                        $$ -> value = -1;

                        /* install the symbol */
                        $$ -> symbol = Insert($$->name, $$->my_data_type, SYM_ARRAY, LEVEL+1, 1, OFFSET);
                        OFFSET = OFFSET + 1;
                     }/* use value 0 and -1 to determin it has [] after or not in ast,c */
                    ; /* 10. param → type-specifier ID [ [] ]+ */
                    /* created A_PARAMDEC node */


Compound_Stmt       : '{' 
                      {
                        LEVEL++;
                      } 
                      Local_Declaration Statement_List  '}' 
                      {
                         $$ = ASTCreateNode(A_COMPOUND);
                         $$ -> s1 = $3;
                         $$ -> s2 = $4;
                         if(mydebug) Display(); /* added mydebug on lab 9 */
                         // we set the maxoffset
                         if(OFFSET > maxoffset) maxoffset = OFFSET;
                         OFFSET -= Delete(LEVEL);
                         LEVEL--;
                      } /* added on lab6 */
                    ; /* 11. compound-stmt → {  local-declarations  statement-list  } */
                    /* created A_COMPOUND node*/
                    /* added level inside of compound, and also added Display and level-- inside of synatx semantic action */
 
Local_Declaration   : /* empty */ 
                     {
                      $$ = NULL;
                     } /* added on lab6, empty means nothing */
                    | Var_Declaration Local_Declaration
                     {
                      $$ = $1;
                      $$ -> next = $2;
                     } /* added on lab6, use next to connect */
                    ; /* 12. local-declarations → { var-declaration } */ 

Statement_List      : /* empty */ 
                      {
                        $$ = NULL;
                      } /* added on lab6 */
                    | Statement Statement_List
                      {
                        $$ = $1;
                        $$ -> next = $2;
                      } /* added on lab6 */
                    ; /* 13. statement-list → { statement } */ 
                    /* use next to connect */

Statement           : Expression_Stmt { $$ = $1;}
                    | Compound_Stmt { $$ = $1;}
                    | Selection_Stmt { $$ = $1;} /* changed on Mar 10 */
                    | Iteration_Stmt { $$ = $1;}
                    | Assignment_Stmt { $$ = $1;}
                    | Return_Stmt { $$ = $1;} /* changed on Mar 11 */
                    | Read_Stmt { $$ = $1;} /* modified on Mar 11 */
                    | Write_Stmt { $$ = $1;} /* temoprary added on lab6, needs to fix it, has fixed it on Mar 11 */
                    | Break_Stmt { $$ = $1;} /* added on lab 9 */
                    ; /* 14. statement → expression-stmt | compound-stmt | selection-stmt | iteration-stmt | assignment-stmt | return-stmt | read-stmt | write-stmt */


Break_Stmt          : T_BREAK ';'
                      {
                        $$ = ASTCreateNode(A_BREAK);
                        $$ -> value = 0;
                        $$ -> linecount = lineno; // store line number into PTR for emit.c
                      }
                    | T_CONTINUE ';'
                      {
                        $$ = ASTCreateNode(A_BREAK);
                        $$ -> value = -1;
                        $$ -> linecount = lineno; // store line number into PTR for emit.c
                      }
                    ;

Expression_Stmt     : Expression ';'
                      {
                        $$ = ASTCreateNode(A_EXPRSTMT);
                        $$ -> s1 = $1;
                      }
                    | ';'
                      {
                        $$ = ASTCreateNode(A_EXPRSTMT);
                        $$ -> s1 = NULL;
                      } /* added on Mar 11 */
                      /* use s1 to determin the difference in ast.c */
                    ; /* 15. expression-stmt → expression ; | ; */
                    /* created A_EXPRSTMT node */

Selection_Stmt      : T_IF '(' Expression ')' Statement
                      {
                        $$ = ASTCreateNode(A_SELESTMT);
                        $$ -> s1 = $3;
                        $$ -> s2 = ASTCreateNode(A_SELEBODY);
                        $$ -> s2 -> s1 = $5;
                        $$ -> s2 -> s2 = NULL;
                      } /* assign s2->s2 is null to determin the difference in ast.c */
                    | T_IF '(' Expression ')' Statement T_ELSE Statement
                      {
                        $$ = ASTCreateNode(A_SELESTMT);
                        $$ -> s1 = $3;
                        $$ -> s2 = ASTCreateNode(A_SELEBODY);
                        $$ -> s2 -> s1 = $5;
                        $$ -> s2 -> s2 = $7;
                      } /* added both on Mar 11 */
                     
                    ; /* 16. selection-stmt → if ( expression ) statement [ else statement ] + */
                    /* this case needs to create 2 nodes (a nested node inside 1st node), 1st is for T_IF + expression, and 2nd is for body */
                    /* the only difference is the "if else" has 2nd stmt */

Iteration_Stmt      : T_WHILE '(' Expression ')' Statement
                      {
                        $$ = ASTCreateNode(A_WHILE);
                        $$ -> s1 = $3;
                        $$ -> s2 = $5;
                      } /* added on Mar 11 */
                    ; /* 17. iteration-stmt → while ( expression ) statement  */
                    /* created A_WRITE node */

Return_Stmt         : T_RETURN ';'
                      {
                        $$ = ASTCreateNode(A_RETRUNSTMT);
                      }
                    | T_RETURN Expression ';'
                      {
                        $$ = ASTCreateNode(A_RETRUNSTMT);
                        $$ -> s1 = $2;
                      } /* both of them added on Mar 11 */
                    ; /* 18. return-stmt → return [ expression ]+ ; */
                    /* created A_RETRUNSTMT node */
                    /* use ->s1 is null or not to determin the difference in ast.c */


Read_Stmt           : T_READ Var ';'
                      {
                        $$ = ASTCreateNode(A_READ);
                        $$ -> s1 = $2;
                      } /* added on Mar 11 */
                    ; /* 19. read-stmt → read var ; */
                    /* created A_READ node */
 


Write_Stmt          : T_WRITE Expression ';' 
                      {
                        $$ = ASTCreateNode(A_WRITE);
                        $$ -> s1 = $2;
                      } /* added on Mar 6 */
                    | T_WRITE T_STRING ';' 
                      {
                        $$ = ASTCreateNode(A_WRITE);
                        $$ -> name = $2;
                      } /* added on lab6 */
                    ; /* 20. write-stmt →  write expression; | write string; */
                    /* created A_WRITE node */
                    /* for write expr, extends to s1, but just stores the string to ->name for write string */ 

Assignment_Stmt     : Var '=' Simple_Expression ';'
                      { 
                        if($1->my_data_type != $3->my_data_type)
                        { /* check the data type */ 
                          yyerror("Type mismatch in ASSIGN");
                          exit(1);
                        } 
                        $$ = ASTCreateNode(A_ASSISTMT);
                        $$ -> s1 = $1;
                        $$ -> s2 = $3;
                        $$ -> name = CreateTemp();
                        $$ -> symbol = Insert($$->name, $3->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                        OFFSET = OFFSET + 1; /* added on mar 28, to create a temoprary symbol for Assignment_Stmt */
                      } /* added on Mar 11 */
                    ; /* 21. assignment-stmt →  var = simple-expression   ;   */
                    /* created A_ASSISTMT node */



Var                 : T_ID
                      {
                        struct SymbTab *p;
                        p = Search($1, LEVEL, 1);
                        if(p == NULL)
                        { /* the var is not defined */
                          yyerror($1);
                          yyerror("symbol used but not defined");
                          exit(1);
                        }
                        if(p->SubType != SYM_SCALAR)
                        {
                          /* the var is not a scalar */
                          yyerror($1);
                          yyerror("symbol is not a scalar");
                          exit(1);
                        }
                        $$ = ASTCreateNode(A_VAR);
                        $$ -> name = $1;
                        $$ -> value = -1;
                        $$ -> symbol = p; /* added on lab 7, need to fix others*/
                        $$ -> my_data_type = p -> Declared_Type; /* added for type checking, need to fix others */
                        $$ -> symbol -> name = $1; /* to store the name to symtable */
                      } /* added on Mar 11 */
                    | T_ID '[' Expression ']' 
                      {
                        struct SymbTab *p;
                        p = Search($1, LEVEL, 1);
                        if(p == NULL)
                        { /* the var is not defined */
                          yyerror($1);
                          yyerror("symbol used but not defined");
                          exit(1);
                        }
                        if(p->SubType != SYM_ARRAY)
                        {
                          /* the var is not a scalar */
                          yyerror($1);
                          yyerror("symbol is not an array");
                          exit(1);
                        } /* added on mar 26 */

                        $$ = ASTCreateNode(A_VAR);
                        $$ -> name = $1;
                        $$ -> s1 = $3;
                        $$ -> value = 0;
                        $$ -> symbol = p; /* added on lab 7, to store the pointer to the p-> symbol */
                        $$ -> symbol -> name = $1;
                        $$ -> my_data_type = p -> Declared_Type; 
                        $$ -> symbol -> name = $1; /* to store the name to symtable */
                      } /* added on Mar 11 */
                    ; /* 22. var → ID  [ [ expression ]  ] + */
                    /* created A_VAR node */
                    /* also use ->value 0 and -1 to determin the difference in ast.c */


Expression          : Simple_Expression
                      {
                        $$ = $1;
                      } /* added on Mar 11 */
                    ; /* 23. expression → simple-expression  */


Simple_Expression   : Addictive_Expression
                      {
                        $$ = $1;
                      } /* added on Mar 11 */
                    | Addictive_Expression Relop Addictive_Expression
                      {
                        if($1->my_data_type != $3->my_data_type)
                        { /* check the data type */ 
                          yyerror("Type mismatch additive");
                          exit(1);
                        } 
                        $$ = ASTCreateNode(A_EXPR);
                        $$ -> operator = $2;
                        $$ -> s1 = $1;
                        $$ -> s2 = $3;
                        $$ -> name = CreateTemp();
                        $$ -> symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                        OFFSET = OFFSET + 1; /* added on mar 28, to create a temoprary symbol for args */
                        $$ -> my_data_type = $1 -> my_data_type; /* added on lab 7 for type checking */
                      } /* added on Mar 11 */
                    ; /* 24. simple-expression → additive-expression [ relop additive-expression ] + */
                    /* created A_EXPR node, I originally divide relop and other operators seperately, but combined them as A_EXPR is better for next lab */
                    /* save operators to ->operator for ast.c's switch */


Relop               : T_LE
                      {
                        $$ = A_LESSEQUAL;
                      }
                    | T_L
                      {
                        $$ = A_LESS;
                      }
                    | T_B
                      {
                        $$ = A_BIG;
                      }
                    | T_BE
                      {
                        $$ = A_BIGEQUAL;
                      }
                    | T_E 
                      {
                        $$ = A_EQUAL;
                      }
                    | T_NE 
                      {
                        $$ = A_NOTEQUAL;
                      }
                    ; /* 22. relop → <= | < | > | >= | == | != */
                    /* assigned them to c use define operators */
                    /* added above to ast operator in ast.h */


Addictive_Expression: Term
                      {
                        $$ = $1;
                      } /* added on Mar 6 */
                    | Addictive_Expression Addop Term
                      {
                        if($1->my_data_type != $3->my_data_type)
                        { /* check the data type */ 
                          yyerror("Type mismatch additive");
                          exit(1);
                        } 
                        $$ = ASTCreateNode(A_EXPR);
                        $$ -> operator = $2;
                        $$ -> s1 = $1;
                        $$ -> s2 = $3;
                        /* added on lab 7, still feel confusied */
                        $$ -> name = CreateTemp();
                        $$ -> symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                        OFFSET = OFFSET + 1;
                        $$ -> my_data_type = $1 -> my_data_type; /* added on lab 7 for type checking */
                      } /* added on Mar 6 */
                    ; /* 23. additive-expression → term { addop term } */
                    /* also created A_EXPR node */


Addop               : T_PLUS
                      {
                        $$ = A_PLUS;

                      } /* added on March 6 */
                    | T_MINUS
                      {
                        $$ = A_MINUS;

                      } /* added on March 6 */
                    ; /* 24. addop → + | - */
                    /* assigned to c user define operators */
                    /* added operator to ast_operator in ast.h for astprint */

Term                : Factor {$$ = $1;} /* added on Mar 6 */
                    | Term Multop Factor
                      {
                        if($1->my_data_type != $3->my_data_type)
                        { /* check the data type */ 
                          yyerror("Type mismatch term");
                          exit(1);
                        } 
                        $$ = ASTCreateNode(A_EXPR);
                        $$ -> operator = $2;
                        $$ -> s1 = $1;
                        $$ -> s2 = $3;
                        $$ -> name = CreateTemp();
                        $$ -> symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                        OFFSET = OFFSET + 1; /* added on mar 28, to create a temoprary symbol for term */
                        /* need to add something */
                        $$ -> my_data_type = $1 -> my_data_type; /* added on lab 7 for type checking */
                      } /* added on Mar 11 */
                    ; /* 25. term → factor { multop  factor } */
                    /* created A_EXPR node, same with previous */
                    /* store multop to ->operator for the switch of astprint in ast.c */
 

Multop              : T_MUTIPLE
                      {
                        $$ = A_MUTIPLE;
                      }
                    | T_DIVIDE
                      {
                        $$ = A_DIVIDE;
                      } /* added on Mar 11 */
                    | T_MOD
                      {
                        $$ = A_MOD;
                      } /* added on Mar 11 */
                    ; /* 26. multop →  * | / */
                    /* assigned to c user define operators */

Factor              : '(' Expression ')' 
                     {
                      $$ = $2;
                     } /* added Mar 6 */
                    | T_NUM
                      {
                        $$ = ASTCreateNode(A_NUM);
                        $$ -> value = $1;
                        $$ -> my_data_type = A_INTTYPE; /* added on lab 7 for type checking */
                      } /* added on Mar 6 */
                    | Var 
                      {
                        $$ = $1;
                      } /* added on Mar 6 */
                    | Call
                      {
                        $$ = $1;
                      } /* modified on Mar 11 */
                    | T_MINUS Factor
                      {
                        if($2->my_data_type != A_INTTYPE)
                        {
                          yyerror("Type mismatch uminus");
                          exit(1);
                        } // for type checking
                        $$ = ASTCreateNode(A_EXPR);
                        $$ -> operator = A_UMINUS;
                        $$ -> s1 = $2;
                        $$ -> my_data_type = A_INTTYPE;
                        $$ -> name = CreateTemp();
                        $$ -> symbol = Insert($$->name, $2->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                        OFFSET = OFFSET + 1; /* added on mar 28, to create a temoprary symbol for uminus */
                      } /* added on Mar 11 */
                    ; /* 27. factor → ( expression ) | NUM |  var | call  | - factor */
                    /* aggain, created A_EXPR node for all operators */
 

Call                : T_ID '(' Args ')' 
                      {
                        struct SymbTab *p;
                        p = Search($1, 0, 0);

                        if(p==NULL)
                        {
                          /*function name unknown */
                          yyerror($1);
                          yyerror("function name not defined");
                          exit(1);
                        }
                        /* name is there but is it a function */
                        if(p->SubType != SYM_FUNCTION)
                        {
                          yyerror($1);
                          yyerror("is not defined as a function");
                          exit(1);
                        }
                        /* checked the formal and actual param are same length and type */
                        if(check_params($3, p->fparms)==0)
                        {
                          yyerror($1);
                          yyerror("Actual and Formals do not match");
                          exit(1);
                        }

                        $$ = ASTCreateNode(A_CALL);
                        $$ -> name = $1;
                        $$ -> s1 = $3;
                        $$ -> symbol = p; /* added on lab 7 */
                        $$ -> my_data_type = p -> Declared_Type;/* added on lab 7 */
                      } /* added on Mar 11 */
                    ; /* 28. call → ID ( args ) */
                    /* created A_CALL node*/

Args                : Arg_List
                      {
                        $$ = $1;
                      } /* added on Mar 11 */
                    | /* empty */
                      {
                        $$ = NULL;
                      } /* added on Mar 11 */
                    ; /* 29. args → arg-list | empty */

Arg_List            : Expression
                      {
                        $$ = ASTCreateNode(A_ARGLIST);
                        $$ -> s1 = $1;
                        $$ -> name = CreateTemp();
                        $$ -> symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                        OFFSET = OFFSET + 1; /* added on mar 28 */
                        $$ -> my_data_type = $1 -> my_data_type; /* save data type to ptr */
                      }
                    | Expression ',' Arg_List
                      {
                        $$ = ASTCreateNode(A_ARGLIST);
                        $$ -> s1 = $1;
                        $$ -> next = $3;
                        $$ -> name = CreateTemp();
                        $$ -> symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
                        OFFSET = OFFSET + 1; /* added on mar 28, to create a temoprary symbol for args */
                        $$ -> my_data_type = $1 -> my_data_type; /* save data type to ptr */
                      } /* added on Mar 11 */
                    ; /* 30. arg-list → expression { , expression } */
                    /* created A_ARGLIST node and used next to connect */



%% /* end of rules, start of program */
int main(int argc, char *argv[])
{ 
  FILE *fp;
  // read our input arguments
  int i;
  char s[100];
  
  // option -d turn on debug
  // option -o next argument is our output file name
  for(i = 0; i < argc; i++)
  { 
    if(strcmp(argv[i], "-d") == 0) mydebug = 1;
    if(strcmp(argv[i], "-o") == 0) 
    {
      // we have a file input
      strcpy(s, argv[i + 1]);
      strcat(s, ".asm");
    
      // now open the file that is referenced by s
      fp = fopen(s, "w");
      if(fp == NULL)
      {
        printf("cannot open file ;%s; \n", s);
        exit(1);
      } // end of if fp
    } // end of if -o
  }// end of for loop

  yyparse();
  if (mydebug) printf("\nFinished Parsing\n\n\n"); /* added on lab6 */
  if (mydebug) Display(); /* show our global variable function */
  if (mydebug) printf("\n\nAST PRINT\n\n");
  if (mydebug) ASTprint(0, PROGRAM); /* added on lab6 */ 

  EMIT(PROGRAM, fp); /* added on lab 9 */
}
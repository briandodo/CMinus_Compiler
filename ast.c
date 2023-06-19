
/* 
* Name: Meng-Yang Tseng
* Date: April 10 2023
* lab: lab9
* purpose: create abstract syntax tree based on pointer from lab6.y's grammar rules

* changed(lab6):
            * I fixed void PT(int howmany) and char * ASTtypeToString(enum ASTtype mytype)
            * created cases(the node I created in lab6.y ) in switch of ASTprint
* changed(lab7):
            * added level %d offset %d into function declaration, var declaration, and var
            * added search params function to check params
* changed(lab9):
            * deleted A_SELEBODY case inside of switch to make emit.c to implement easier
            * added case A_BREAK to deal with break and continue stmt
*/

#include<stdio.h>
#include<malloc.h>
#include "ast.h" 


/* uses malloc to create an ASTnode and passes back the heap address of the newley created node */
//  PRE:  Ast Node Type
//  POST:   PTR To heap memory and ASTnode set and all other pointers set to NULL
ASTnode *ASTCreateNode(enum ASTtype mytype)

{
    ASTnode *p;
    if (mydebug) fprintf(stderr,"Creating AST Node \n");
    p=(ASTnode *)malloc(sizeof(ASTnode));
    p->type=mytype;
    p->s1=NULL;
    p->s2=NULL;
    p->next=NULL;
    p->value=0;
    
    return(p);
}

/*  Helper function to print tabbing */
//PRE:  Number of spaces desired
//POST:  Number of spaces printed on standard output

void PT(int howmany)
{
	for (int i = 0; i < howmany; i++) {
        printf(" ");
    }
    // MISSING
    // Mar 13 updated, I have fixed this missing part */
}

//  PRE:  A declaration type
//  POST:  A character string that is the name of the type
//          Typically used in formatted printing
char * ASTtypeToString(enum ASTtype mytype)
{
    
    switch (mytype)
    {
    case A_INTTYPE:
        return "INT";
        break;
    case A_VOIDTYPE:
        return "VOID";
        break;
    default:
        return "Unknown type";
        break;
    }
    // Missing
    // Mar 12 Updated, I have fixed it by switch stmt

}



/*  Print out the abstract syntax tree */
// PRE:   PRT to an ASTtree
// POST:  indented output using AST order printing with indentation

void ASTprint(int level,ASTnode *p)
{
   int i;
   if (p == NULL ) return;
   /* delete else*/
       switch (p->type) {
        case A_VARDEC : 
                        PT(level); 
                        if (p->value==0)
                        {
                            printf("Variable %s %s level %d offset %d\n", ASTtypeToString(p -> my_data_type), p->name, p->symbol->level, p->symbol->offset); /* added on lab6*/
                        } //for the var without []
                        // added level and offset on lab 7, need to fix proto
                        else
                        {
                            printf("Variable %s %s[%d] level %d offset %d\n", ASTtypeToString(p -> my_data_type), p->name, p->value, p->symbol->level, p->symbol->offset); /* added on Mar 12*/
                        } //for the var with []
                        //if value equla to 0, it means var declaration doesn't have [] after
                        ASTprint(level, p -> s1); /* for Var_List*/
                        ASTprint(level, p -> next); /* added on lab6*/
                        break;
                        /* for A_VARDEC, the if else statement is for int x[2] and int x, if it is the 1st case, it will print extra value*/
        case A_FUNCTIONDEC :  
                        PT(level); 
                        if (p->s2!=NULL)
                        {
                            printf("Function %s %s level %d offset %d\n", ASTtypeToString(p -> my_data_type),p -> name, p->symbol->level, p->symbol->offset);  /* added on lab6 */
                        }// for function declartion with compound stmt
                        else
                        {
                            printf("Function Prototype %s %s\n", ASTtypeToString(p -> my_data_type),p -> name); /* added on Mar 12*/
                        }//I use p->value to save value 1 to indicate the function declartion is prototype or not
                        // 1 means not, the other is yes
                        ASTprint(level + 1, p -> s1);//for Params
                        ASTprint(level + 1, p -> s2);//for Compound_Stmt 
                        ASTprint(level, p -> next);// for the next function
                        break;
                        /* for A_FUNCTIONDEC, the if else statement is for prototype and normal function, I used value to help computer to determin*/
        case A_COMPOUND :
                        PT(level); printf("Compund Statement \n");  /* added on lab6 */
                        ASTprint(level + 1, p -> s1);// for Local_Declaration
                        ASTprint(level + 1, p -> s2);// for Statement_List
                        ASTprint(level, p->next);// added p->next becasue it is necessary
                        break;
                        /* for compund statement*/
        case A_WRITE : /* added on lab6 */
                        PT(level);
                        if (p -> name != NULL)
                            {
                                printf("Write String %s \n", p -> name);
                            }// for write "test" case
                        else 
                            {
                                printf("Write Expression\n");
                                ASTprint(level+1, p->s1);//p->s1 recursion, because it has expression which is in s1
                            } /* added on Mar 6, for write expression like 1 + 1*/
                        //Write can be string or expression, so I save the string to p->name
                        //if p->name is not NULL, it means writing string. On the other hand, it is write expression
                        ASTprint(level, p -> next); /* added on Mar 10*/
                        break;
                        /* for write, because it doesn't have s1 s2, it doesn't need to create recursion for those cases*/
        case A_READ:    
                        PT(level);
                        printf("READ STATEMENT\n");
                        ASTprint(level + 1, p->s1);//for Var
                        ASTprint(level + 1, p->next);// added on mar 27 
                        break; /* added on Mar 11*/
                        /* for read, it has s1*/
        case A_NUM : 
                        PT(level);
                        printf("NUMBER value %d \n", p->value);
                        break;
                        /* for factor's T_NUM*/
        case A_EXPR :   PT(level);
                        switch (p->operator)
                        {
                            case A_PLUS:// for +
                                printf("EXPRESSION operator PLUS\n");
                                break; 
                            case A_MINUS://for -
                                printf("EXPRESSION operator MINUS\n");
                                break;
                            case A_LESSEQUAL://for <=
                                printf("EXPRESSION operator <=\n");
                                break;
                            case A_LESS://for <
                                printf("EXPRESSION operator <\n");
                                break;
                            case A_BIG://for >
                                printf("EXPRESSION operator >\n");
                                break;
                            case A_BIGEQUAL://for >=
                                printf("EXPRESSION operator >=\n");
                                break;
                            case A_EQUAL://for ==
                                printf("EXPRESSION operator ==\n");
                                break;
                            case A_NOTEQUAL://for !=
                                printf("EXPRESSION operator !=\n");
                                break;
                            case A_UMINUS://for -x case
                                printf("EXPRESSION operator Unary-minus\n");
                                break;
                            case A_MUTIPLE:// for *
                                printf("EXPRESSION operator TIMES\n");
                                break;
                            case A_DIVIDE://for /
                                printf("EXPRESSION operator DIVIDE\n");
                                break;
                            case A_MOD:// for %
                                printf("EXPRESSION operator %\n");
                                break;
                            case A_ASSISTMT: // for =
                                
                            default:// default, if it is not included 
                                printf("Unknown operactor in A_EXPR in ASTprint\n");

                            
                                
                        } //I save all operators into p->operator, and use switch to determin which kind of operator
                        ASTprint(level + 1, p->s1);//for the value before operator
                        ASTprint(level + 1, p->s2);// for the value after operator
                        break;
                        /* for expression*/
        case A_PARAMDEC:
                        PT(level); 
                        if (p -> value == 0)
                        {
                            printf("Parameter %s %s level %d offset %d\n", ASTtypeToString(p -> my_data_type), p -> name, p->symbol->level, p->symbol->offset);
                        } /* for non [] case*/
                        else
                        {
                            printf("Parameter %s %s[] level %d offset %d\n", ASTtypeToString(p -> my_data_type), p -> name, p->symbol->level, p->symbol->offset);
                        } /* for array case*/
                        // for paramdec, there are [] after and nothing after, so I use p->value as boolean to
                        //determin two situation, 0 means no [] after
                        ASTprint(level, p->next);//for the next
                        break; /* added on Mar 10*/
                        /* for parameter dec*/
        case A_SELESTMT: 
                        PT(level);
                        printf("IF STATEMENT\n");
                        PT(level + 1);// level + 1 
                        printf("IF expression\n");
                        ASTprint(level + 2, p->s1);//for the expression
                        PT(level + 1);// level + 1 
                        printf("IF body\n");
                        ASTprint(level + 2, p->s2->s1);//for the stmt (1st)
                        if(p->s2->s2 != NULL)
                        {
                            PT(level + 1);// level + 1 
                            printf("ELSE body\n");
                            ASTprint(level + 2, p->s2->s2);//for the stmt (2st)
                        }
                        ASTprint(level, p->next);// for the next
                        break;
                        /* for if else stmt's expression part*/
        case A_RETRUNSTMT:
                        PT(level);
                        printf("RETURN STATEMENT\n");
                        if(p->s1 != NULL)
                        {
                            ASTprint(level + 1, p->s1);// for the expression
                        }
                        ASTprint(level, p->next);// for the next
                        // because it can return value or nothing, so I use if to determin 
                        //if p->s1 is not null, it means there is value after return
                        break; /* added on Mar 11*/
                        /* for return stmt*/
        case A_VAR:     
                        PT(level);
                        if (p -> value == -1)
                        {
                            printf("VARIABLE %s level %d offset %d\n", p->name, p->symbol->level, p->symbol->offset);// for var without []
                        }
                        else
                        {
                            printf("VARIABLE %s level %d offset %d\n", p->name, p->symbol->level, p->symbol->offset);// modified on lab 7
                            PT(level);
                            printf(" [\n");
                            ASTprint(level + 2, p->s1);
                            PT(level);
                            printf(" ]\n");// for var with []
                        } // in this case, there are var and var[], so I use p->value to store -1 as boolean 
                        //to determine it needs to print [] after or not
                        break; /* added on Mar 11*/
                        /* for variable which is inside of call*/
        case A_EXPRSTMT:
                        PT(level);
                        printf("Expression STATEMENT\n");// for expr stmt
                        ASTprint(level + 1, p->s1);//for Expression
                        ASTprint(level, p->next);
                        break; /* added on Mar 11*/
                        /* for expression stmt*/
        case A_CALL:    
                        PT(level);
                        printf("CALL STATEMENT  function %s\n", p->name);
                        PT(level + 1);
                        printf("(\n");
                        ASTprint(level + 1, p->s1);// for Args inside of call()
                        PT(level + 1);
                        printf(")\n");
                        break; /* added on Mar 11*/
                        /* for call stmt*/
        case A_ARGLIST: 
                        PT(level + 1);
                        printf("CALL argument\n");// for args inside of call()
                        ASTprint(level + 2, p->s1);//for Expression
                        ASTprint(level, p->next);//for next arg
                        break; /* added on Mar 11*/
                        /* for arg_list*/
        case A_WHILE:   
                        PT(level);
                        printf("WHILE STATEMENT\n");
                        PT(level + 1);
                        printf("WHILE expression\n");
                        ASTprint(level + 2, p->s1);// for expression of while
                        PT(level + 1);
                        printf("WHILE body\n");
                        ASTprint(level + 2, p->s2);// for the Statement
                        ASTprint(level, p->next);// for the next
                        break; /* added on Mar 11*/
                        /* for while stmt*/
        case A_ASSISTMT: 
                        PT(level);
                        printf("ASSIGMENT STATEMENT\n");
                        ASTprint(level + 1, p->s1);// for the value which is assigned
                        PT(level);
                        printf("is assigned\n");
                        ASTprint(level + 1, p->s2);// for the expr after =
                        ASTprint(level, p->next);// for the next
                        break; /* added on Mar 11*/
                        /* for assignment stmt*/
        case A_BREAK:   
                        PT(level);
                        if(p->value == 0)
                        {
                            printf("Break STATEMENT\n");
                        }
                        else
                        {
                            printf("Continue STATEMENT\n");
                        }
                        ASTprint(level, p->next);// for the next
                        break;

        default: printf("unknown AST Node type %d in ASTprint\n", p->type);// default, when it doesn't have matched case


       }
     

}//end of astprint

// PRE: PTRS to actual and formals
// Post: 0 if they are not same type or length
//       1 if they are
int check_params(ASTnode *actuals, ASTnode *formals)
{
    if(actuals == NULL && formals == NULL) return 1;
    if(actuals == NULL || formals == NULL) return 0;
    if(actuals->my_data_type != formals->my_data_type) return 0;//when actula and formal are different

    return check_params(actuals->next, formals->next); // recursion to check 
    // need to fix it
}



/* dummy main program so I can compile for syntax error independently   
main()
{
}
/* */

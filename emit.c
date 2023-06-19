/*
* level f for grading
* emit.c
* Name : Meng-Yang Tseng
* Date : April 11 2023
* Purpose : 
            Program that uses AST which created on lab 6 to generate MIPS code,
            EMIT() function is the base function, and it has .data, .align2, and .text part, and 
            I deal with string, global variable and ast based on EMIT(), and created functions for every
            node. Finally it outputs MIPS code which can work
*/

#include "ast.h"
#include "emit.h"
#include <string.h> /* for strdup*/
#include <stdlib.h> /* for exit*/
#include <stdio.h> /* for stack*/

int GLABEL=0;/* global variable for label counter, not sure it should be global or not*/
int GPARAM=0;//global register counter for parameter
ASTnode * CHECK; // global PTR to check it is main function or not
struct Stack* GSTACK;//global stack to store stack

// PRE: PTR to AST, PTR File
// POST: prints out MIPS code into file, using helper functions
void EMIT(ASTnode *p, FILE * fp)
{
    if (p == NULL) return;
    if (fp == NULL) return;

    fprintf(fp, "# MIPS CODE GENERATE by Compiler class\n\n");
    fprintf(fp, ".data\n\n");
    EMIT_STRINGS(p, fp);// for write string
    fprintf(fp, ".align 2\n");
    EMIT_GLOBALS(p, fp);// for global variable
    fprintf(fp, ".text\n\n\n");
    fprintf(fp, ".globl main\n\n\n");//global main, it should print out every time
    EMIT_AST(p,fp);//for other node of ast
}//end of EMIT

// PRE:  PTR to top of AST, and FILE ptr to print to 
// POST:  Adds a label into the AST for use string in write statements 
// POST:  prints MIPS based  string  into file 
void EMIT_STRINGS(ASTnode * p, FILE * fp){
    if(p==NULL) return;// when pointer is null
    if(fp==NULL) return;// when file is null, not sure it needs it or not
    if (p->type == A_WRITE && p->name != NULL)
    {   // when node type is A_WRITE and name is not null (means write string)
        // create a label and assign to p->label
        // finally print to the .asm file
        p->label = CreateLabel();
        fprintf(fp, "%s: .asciiz	 %s\n", p->label, p->name);
    }
    EMIT_STRINGS(p->s1, fp);
    EMIT_STRINGS(p->s2, fp);
    EMIT_STRINGS(p->next, fp);//recursive
    
}// end of emit string

// PRE: Any write with string
// POST: Returns string with the format _L%d and increment the global variable
// USAGE: create a label name that is used to hold string of write
char * CreateLabel()
{
  
  char hold[100];
  char *s;
  sprintf(hold, "_L%d", GLABEL++);//store value to char hold
  s=strdup(hold);//create a copy of a null-terminated string and store to PTR
  return(s);//return PTR s

}

// PRE:  PTR to top of AST, and FILE ptr to print to 
// POST:  prints MIPS based  global variables into file
// USAGE: when there is global var, it needs a label and print on the header of .asm file 
void EMIT_GLOBALS(ASTnode * p, FILE * fp)
{
    if(p==NULL) return;//when p is null or fp is null, but not sure 2nd one needs or not
    if(fp==NULL) return;
    if(p->type == A_VARDEC && p->symbol->level == 0)
    {   //carve the offset for global varibale
        fprintf(fp, "%s: .space %d # global variable\n", p->name, p->symbol->mysize * WSIZE);
    }
    EMIT_GLOBALS(p->s1, fp);
    EMIT_GLOBALS(p->s2, fp);
    EMIT_GLOBALS(p->next, fp);//recursion to check other global varibale
}

//PRE: PTR to ASTnode or NULL
//POST: MIPS code into the file for the tree
// USAGE: To traversal the AST, switch the matched node when we need
void EMIT_AST(ASTnode * p, FILE * fp)
{
    if(p==NULL) return;// if PTR is empty, it should return
    switch (p->type)// use node type to switch
    {
        case A_VARDEC:
            EMIT_AST(p->next, fp);//var dec has s1 and next connection, I deal with s1 in emit global
            break;
        case A_FUNCTIONDEC:// emit function uses s1 s2 to deal with param and compound, and uses next to connect next function or vardec
            emit_function(p, fp);
            EMIT_AST(p->next, fp);
            break;
        case A_COMPOUND:
            //no action for s1 vardec already in stack size
            EMIT_AST(p->s2, fp);
            EMIT_AST(p->next, fp);// function uses p->s2 to connect with compound, and compound uses s2 to deal with stmt and next to deal with next compound
            break;
        case A_WRITE:// deal with using our helper function
            emit_write(p, fp);
            EMIT_AST(p->next, fp);//write has next connection
            break;
        case A_READ: // deal with using our helper function
            emit_read(p, fp);
            EMIT_AST(p->next, fp);//read has next connection
            break;
        case A_RETRUNSTMT: // deal with using our helper function
            emit_return(p, fp);
            EMIT_AST(p->next, fp);//return has next connection
            break;
        case A_ASSISTMT: // deal with using our helper function
            emit_assign(p, fp);
            EMIT_AST(p->next, fp);//assign has next connection
            break;
        case A_PARAMDEC:
            emit_param(p, fp);
            EMIT_AST(p->next, fp);//param uses next to connect together
            GPARAM=0;// reset param's counter to 0 every time 
            break;
        case A_SELESTMT:// deal with using our helper funtion for if else
            emit_selet(p, fp);
            EMIT_AST(p->next, fp);//Sele has next connection
            break;
        case A_EXPRSTMT:// deal with using our helper function for exprstmt
            emit_exprstmt(p, fp);
            EMIT_AST(p->next, fp);// EXPR STMT has next connection
            break;
        case A_WHILE:// deal with using our helper function for while loop
            emit_while(p, fp);
            EMIT_AST(p->next, fp);//while has next connection
            break;
        case A_BREAK: // deal with using our helper function for break
            emit_break(p, fp);
            EMIT_AST(p->next, fp);// break is stmt, it has next connection
            break;

        default:
            printf("EMIT AST case %d not implemented \n", p->type);
            printf("WE SHOULD NEVER BE HERE\n");
            exit(1);
    }
}// end of emit ast()

//PRE : possible label, commend, comment
//POST: formatted output to the file
void emit(FILE *fp, char* label, char* commend, char* comment)
{
    if(strcmp("", comment) == 0)
    {   // if comment is null
        if(strcmp("", label) == 0)
        {   // if label is null
            fprintf(fp, "\t%s\t\t\n", commend);
        }
        else
        {   // if label is not null
            fprintf(fp, "%s:\t%s\t\t\n", label, commend);
        }
    }
    else
    {   // if comment is not null
        if(strcmp("", label) == 0)
        {   //if label is null
            fprintf(fp, "\t%s\t\t# %s\n", commend, comment);
        }
        else
        {   // if label is not null
            fprintf(fp, "%s:\t%s\t\t# %s\n", label, commend, comment);
        }
    }
    
    
}// end of emit

//PRE PTR to ASTnode A_FUNCTIONDEC
//POST MIPS code in fp
//Usage : when there is a function, it needs emit_function to create a block for it
void emit_function(ASTnode *p, FILE * fp)
{
    CHECK = p; //assign PTR of current function to CHECK for emit_return
    
    char s[100]; // to do dynamic print
    emit(fp, p->name, "", "function definition");
    //Carve out the Stack for activation record
    emit(fp, "", "move $a1, $sp", "Activation Record carve out copy SP");
    sprintf(s, "subi $a1, $a1, %d", p->symbol->offset * WSIZE);
    emit(fp, "", s, "Activation Record carve out copy size of function");
    emit(fp, "", "sw $ra , ($a1)", "Store Return address");
    sprintf(s, "sw $sp %d($a1)", WSIZE);
    emit(fp, "", s, "Store the old Stack pointer");
    emit(fp, "", "move $sp, $a1", "Make SP the current activation record");
    fprintf(fp, "\n\n");
    //copy the parameters to the formal from registers $t0
    EMIT_AST(p->s1, fp);//extend to param list
    fprintf(fp, "\n\n");//space after paramater
    //generate the compound statment
    GSTACK = createStack();//create stack 
    EMIT_AST(p->s2, fp);//extend to the code inside of compound
    //create an implicit return depending on if we
    //are main or not
    //restore RA and SP before we retur
    emit(fp, "", "li $a0, 0", "RETURN has no specified value set to 0"); // default return, should print to file every time
    emit(fp, "", "lw $ra ($sp)", "restore old environment RA");
    sprintf(s, "lw $sp %d($sp)", WSIZE);
    emit(fp, "", s, "Return from function store SP");
    fprintf(fp, "\n");

    if(strcmp(p->name, "main") == 0)// when function name is main
    {
        //exit the system
        emit(fp, "", "li $v0, 10", "Exit from Main we are done");
        emit(fp, "", "syscall", "Exit everything");
        fprintf(fp, "\n");
        emit(fp, "", "", "END OF FUNCTION");
        fprintf(fp, "\n\n\n");//white space in the end of function
    }
    else
    {   // when function name is not "main"
        //jump back to the caller
        emit(fp, "", "jr $ra", "return to the caller");//return to the caller
        fprintf(fp, "\n");
        emit(fp, "", "", "END OF FUNCTION");
        fprintf(fp, "\n\n\n");//white space in the end of function
    }
}

// PRE: PTR to expression family
//POST: MIPS code that sets $a0 to the value of the expression
// Usage : any expression should use emit_expr directly, cannot call the function which is inside of emit_expr
void emit_expr(ASTnode *p, FILE * fp)
{
    char s[100];
    //base
    switch(p->type)
    {
        case A_NUM:
                sprintf(s, "li $a0, %d", p->value);
                emit(fp, "", s, "expression is a constant");
                return;
                break;
        case A_EXPR:
                emit_expr(p->s1, fp);// recursion of s1
                sprintf(s, "sw $a0, %d($sp)", p->symbol->offset*WSIZE);//stored data to char
                emit(fp, "", s, "expression store LHS temporarily");//emit to the file
                if(p->s2!=NULL)
                {   // for uminus, because uminus's s2 is null
                    emit_expr(p->s2, fp);//recursion of s2
                    emit(fp, "", "move $a1, $a0", "right hand side needs to be a1");
                    sprintf(s, "lw $a0, %d($sp)", p->symbol->offset*WSIZE);//stored data to char
                    emit(fp, "", s, "expression restore LHS from memory");
                }
                switch (p->operator)// switch for all operator
                {
                case A_PLUS:// for +
                    emit(fp, "", "add $a0, $a0, $a1", "EXPR ADD");//print expr add to the file
                    break;
                case A_MINUS:// for -
                    emit(fp, "", "sub $a0, $a0, $a1", "EXPR SUB");
                    break;
                case A_LESSEQUAL:// <=
                    emit(fp, "", "add $a1 ,$a1, 1", "EXPR LE add one to do compare");
	                emit(fp, "", "slt $a0 ,$a0, $a1", "EXPR LE");
                    break;
                case A_LESS:// for <
                    emit(fp, "", "slt $a0, $a0, $a1", "EXPR Lessthan");
                    break;
                case A_BIG:// for >
                    emit(fp, "", "slt $a0, $a1, $a0", "EXPR Greaterthan");
                    break;
                case A_BIGEQUAL:// for >=
                    emit(fp, "", "add $a0 ,$a0, 1", "EXPR  ADD GE");
	                emit(fp, "", "slt $a0, $a1, $a0", "EXPR Greaterthan");
                    break;
                case A_EQUAL:// for ==
                    emit(fp, "", "slt $t2 ,$a0, $a1", "EXPR EQUAL");
	                emit(fp, "", "slt $t3 ,$a1, $a0", "EXPR EQUAL");
	                emit(fp, "", "nor $a0 ,$t2, $t3", "EXPR EQUAL");
	                emit(fp, "", "andi $a0 , 1", "EXPR EQUAL");
                    break;
                case A_NOTEQUAL:// for !=
                    emit(fp, "", "slt $t2 ,$a0, $a1", "EXPR EQUAL");
	                emit(fp, "", "slt $t3 ,$a1, $a0", "EXPR EQUAL");
	                emit(fp, "", "or $a0 ,$t2, $t3", "EXPR EQUAL");
                    break;
                case A_MUTIPLE:// expression "*"
                    emit(fp, "", "mult $a0 $a1", "EXPR MULT");
                    emit(fp, "", "mflo $a0", "EXPR MULT");
                    break;
                case A_DIVIDE:// expression "/"
                    emit(fp, "", "div $a0 $a1", "EXPR DIV");
                    emit(fp, "", "mflo $a0", "EXPR DIV");
                    break;
                case A_UMINUS:// expression negative 
                    emit(fp, "", "neg $a0, $a0", "");
                    // FIX FIX FIX 
                    // need to deal with s2 (UMINUS only has s1)
                    break;
                case A_MOD:// expression "%"
                    emit(fp, "", "div $a0 $a1", "EXPR DIV remainder");
	                emit(fp, "", "mfhi $a0", "EXPR DIV remainder");
                    break;
                
                default:
                    printf("emit operator switch NVER SHOULD BE HERE");
                    printf("FIX FIX FIX");
                    exit(1);//exit 
                }


                break;
        case A_VAR:
                emit_var(p, fp); // $a0 is the memory location
                emit(fp, "", "lw $a0, ($a0)", "Expression is a Var");
                return;
                break;
        case A_CALL:
                emit_call(p, fp);// function for call
                fprintf(fp, "\n\n");// new line after every function
                return;
                break;

        default:
                printf("emit expr switch NEVER SHOULD BE HERE\n");
                printf("FIX FIX FIX\n");
                exit(1);
    }// end of switch
}// end of emit_expr

//PRE: PTR to A_WRITE
//POST: MIPS code to generate WRITE string or write a number
// depending on our argument
// Usage : When there is write node, it can construct MIPS code on the header of .asm, like _L0 : 
void emit_write(ASTnode *p, FILE *fp)
{
    char s[100];
    //if we are writing a string then
    if(p->name != NULL)
    {
        //need to load the address of label into $a0
        //and call print string
        sprintf(s, "la $a0, %s", p->label);//retrieve the label 
        emit(fp, "", s, "The string address ");// emit to .asm
        emit(fp, "", "li $v0, 4", "About to print a string");
        emit(fp, "", "syscall", "call write string");// syscall
        fprintf(fp, "\n\n");// new line after 
         
    }
    else //writing an expression
    {
        emit_expr(p->s1, fp); //now $a0 has the expression value
        emit(fp, "", "li $v0, 1", "About to print a number");
        emit(fp, "", "syscall", "call write number");
        fprintf(fp, "\n\n");
    }
} // end of emit write

//PRE : emit read takes PTR 
// POST : MIPS code to read var
// Usage : when there is read node, it can construct the MIPS code to store var into memory
void emit_read(ASTnode *p, FILE * fp)
{
    emit_var(p->s1, fp); // $a0 will be the location of the variable
    emit(fp, "", "li $v0, 5", "about to read in value");// load immediate
    emit(fp, "", "syscall", "read in value $v0 now has the read in value");
    emit(fp, "", "sw $v0, ($a0)", "store read in value to memory");
    fprintf(fp, "\n\n");// new line after 

}// end of read

// PRE : emit var takes PTR 
// POST : MIPS code depending global variable or local variable
// also array or not array, if it is array, it needs to add internal offset
// Usage : when there is var, it can store it in the SP+offset (local var) or memory (global)
void emit_var(ASTnode *p, FILE *fp)
{
    char s[100];
    
    //handle internal offset if array

    if(p->symbol->level == 0)//global variable
    {
        if(p->s1!= NULL)
        {   // p->s1 is not null = array
            emit_expr(p->s1, fp);// for array
            emit(fp, "", "move $a1, $a0", "VAR copy index array in a1");
            sprintf(s, "sll $a1 $a1 %d", LOG_WSIZE);// combine exponent and string to char s
	        emit(fp, "", s, "muliply the index by wordszie via SLL");
        }
        //get the direct address of global var
        sprintf(s, "la $a0, %s", p->name);//retrieve var name and combine with MIPS code
        emit(fp, "", s, "EMIT Var global variable");
        if(p->s1!=NULL)
        {   // for array, var array type uses s1 for expression
            emit(fp, "", "add $a0 $a0 $a1", "VAR array add internal offset");
        }
        
    }
    else
    { //local variable stack pointer plus offset
        if(p->s1!= NULL)
        {   // p->s1 is not null = array
            emit_expr(p->s1, fp);// for array
            emit(fp, "", "move $a1, $a0", "VAR copy index array in a1");
	        sprintf(s, "sll $a1 $a1 %d", LOG_WSIZE);// combine exponent with char s 
	        emit(fp, "", s, "muliply the index by wordszie via SLL");//emit to file
        }//end of if
        emit(fp, "", "move $a0 $sp", "VAR local make a copy of stackpointer");
        sprintf(s, "addi $a0 $a0 %d", p->symbol->offset*WSIZE);// retrieve offset and combine to char together
        emit(fp, "", s, "VAR local stack pointer plus offset");
        if(p->s1!=NULL)
        {   // for array
            emit(fp, "", "add $a0 $a0 $a1", "VAR array add internal offset");//add internal offset 
        }
    }
    //add on array index if needed
}// end of emit_var()

// PRE : emit return takes PTR 
// POST : MIPS code depending on return empty or expression
// also depends on main function or not, if it is not main function, it should jr $ra to return to the caller
// Usage : when there is return node, it can contruct MIPS code based on function type (main or non-main) and return type (empty or expr)
void emit_return(ASTnode *p, FILE * fp)
{   // need to fix it
    char s[100];
    if(p->s1 != NULL)
    {   // if return is expression
        emit_expr(p->s1, fp);// extend to s1 connection
    }
    else
    {   // when it is just "return" without expression after
        emit(fp, "", "li $a0, 0", "RETURN has no specified value set to 0");
    }
    emit(fp, "", "lw $ra ($sp)", "restore old environment RA");
    sprintf(s, "lw $sp %d($sp)", WSIZE);//combine wsize and MIPS 	
    emit(fp, "", s, "Return from function store SP\n");//emit to file
    if(strcmp(CHECK->name, "main") == 0)
    {   // if return is inside of main function
        emit(fp, "", "li $v0, 10", "Exit from Main we are done");// added on april 
        emit(fp, "", "syscall", "Exit everything");
        
    }
    else
    {   // else when it is not "main" function
        emit(fp, "", "jr $ra", "return to the caller");
        
    }
}// end of emit_return

// PRE : emit assign takes PTR 
// POST : MIPS code to assign p->s2(any expr) to p->s1 (any var)
void emit_assign(ASTnode *p, FILE * fp)
{
    char s[100];
    emit_expr(p->s2, fp);// deal with expression after =
    sprintf(s, "sw $a0, %d($sp)", p->symbol->offset*WSIZE);
    emit(fp, "", s, "Assign store RHS temporarily");
    emit_var(p->s1, fp);//deal with var 
    sprintf(s, "lw $a1 %d($sp)", p->symbol->offset*WSIZE);
    emit(fp, "", s, "Assign get RHS temporarily");
    emit(fp, "", "sw $a1, ($a0)", "Assign place RHS into memory");
}// emit assign

// PRE : emit param takes PTR 
// POST : MIPS code to store to sp+ offset depends on value of offset
// Usage : when there is param, it can store it inside of stack memory
void emit_param(ASTnode *p, FILE * fp)
{
    //static int counter = 0;//counter for register $t0, $t1, $t2...
    char s[100];// char to store the value from pointer
    sprintf(s, "sw $t%d %d($sp)", GPARAM, p->symbol->offset*WSIZE);//get param's offset from pointer and save to char s
    emit(fp, "", s, "Paramater store start of function");//use emit function to print the MIPS code to .asm file
    GPARAM++;//update the number of counter
    
}// end of emit param

//PRE : emit if else takes PTR
//POST : Create two label for postive portion and negative portion of if-else
// also use branch instrution (beq) to indicate which label should it jump to if it is true
// Usage : when there is if else stmt, it can construct a block for it and deal with situation (true false)
void emit_selet(ASTnode *p, FILE * fp)
{
    char * locallabel1;//local label to store value temporary
    char * locallabel2;//local label to store 2nd label value temporary

    locallabel1 = CreateLabel();
    locallabel2 = CreateLabel();

    char s[100]; //char to combine GLABEL and code
    emit_expr(p->s1, fp); //emit condition of if stmt
    sprintf(s, "beq $a0 $0 %s", locallabel1);// store 
    emit(fp, "", s, "IF branch to else part");//emit if rule to file, not sure how to set value for _L
    fprintf(fp, "\n");//new line 
    emit(fp, "", "", "the positive portion of IF");// comment for postive portion of if
    EMIT_AST(p->s2->s1, fp);//emit if stmt
    
    //If s1 end
    sprintf(s, "j %s", locallabel2);//store global label to char s
    emit(fp, "", s, "IF S1 end");//combine char s together and print to file
    //ELSE target
    sprintf(s, "%s", locallabel1);//store if's p->label to char s
    emit(fp, s, "", "ELSE target");//print label and comment to the .asm file
    fprintf(fp, "\n");//new line
    
    emit(fp, "", "", "the negative  portion of IF if there is an else");
	emit(fp, "", "", "otherwise just these lines");// comment for negative portion of if
    EMIT_AST(p->s2->s2, fp);//emit else stmt
    
    //end of if
    sprintf(s, "%s", locallabel2);//store global label to char s
    emit(fp, s, "", "End of IF");// end of if
    fprintf(fp, "\n");//new line 
    
}// end of emit selet

//PRE : emit while takes PTR
//POST : Create two label for MIPS code and use beq to indicate which label should it jump to if condition is true
// beq $a0 $0 _L1 = when a0 equals to $0(always 0), it should jump to _L1
// $a0 is 0 or 1 from condtion
// Usage : when there is while node, it can construct a while block and deal with situation for true or false
void emit_while(ASTnode *p, FILE * fp)
{
    char * locallabel1;//local label to store value temporary
    char * locallabel2;

    //create label for them
    locallabel1 = CreateLabel();
    locallabel2 = CreateLabel();

    push(GSTACK, locallabel1, locallabel2);//insert value to stack

    char s[100];//char s to store value temporary
    sprintf(s, "%s", locallabel1);// for gen
    emit(fp, s, "", "WHILE TOP target");//print "_Lx:			# # WHILE TOP target" to the file
    emit_expr(p->s1, fp);//condition of while
    sprintf(s, "beq $a0 $0 %s", locallabel2);//store global counter value to char s
    emit(fp, "", s, "WHILE branch out");// print "beq $a0 $0 _L2		# #WHILE branch out" to .asm file
    EMIT_AST(p->s2, fp); //while stmt
    sprintf(s, "j %s", locallabel1);//retrieve p->label for jump
    emit(fp, "", s, "WHILE Jump back");//print to .asm file
    sprintf(s, "%s", locallabel2);//combine counter and gen together
    emit(fp, s, "", "End of WHILE");//print to .asm file, end of while
    pop(GSTACK);//remove the top element of stack
    
}// end of emit while

// PRE : emit takes expression statement node
// POST : MIPS code based on emit expression, it can be empty or non empty
// Usage : the bridge between expr stmt and nodes which are under it level
void emit_exprstmt(ASTnode *p, FILE * fp)
{
    if(p->s1!=NULL)
    {
        emit_expr(p->s1, fp);// all expr belongs to expr stmt
    }
}// end of emit_exprstmt

// PRE : emit takes call 
// POST : MIPS code to store arg into memory depends on offset, and pull back to T register when calling the function
// Usage : when there is call node, it can deal with every args and pass args to function
void emit_call(ASTnode *p, FILE *fp)
{
    ASTnode *current1;// for 1st while loop, evaluate function parameter
    ASTnode *current2;// for 1st while loop, parameter into T registers
    current1 = p;
    current2 = p;// assigne PTR to currentx to deal with two while case
    static int counter = 0;// counter for t0, t1, t2...
    char s[100];
    emit(fp, "", "", "Setting Up Function Call");//header for call
    emit(fp, "", "", "evaluate  Function Parameters");
    if(current1->s1!=NULL)// deal with there is arg(one or more)
    {     
        current1 = current1->s1;//move PTR to arglist
        while(current1!=NULL)
        {   // while loop to deal with there is not only 1 arg 
            emit_expr(current1->s1, fp);//deal with expression
            sprintf(s, "sw $a0, %d($sp)", current1->symbol->offset*WSIZE);// retrieve offset from PTR and store to char s
            emit(fp, "", s, "Store call Arg temporarily");
            current1 = current1->next;//move to next node
            fprintf(fp, "\n");//space after every arg
        }
        
    }
    emit(fp, "", "", "place   Parameters into T registers");
    if(current2->s1!=NULL)// deal with there is arg(one or more)
    {   
        current2 = current2->s1;//move PTR to arglist
        while(current2!=NULL)
        {   // while loop to deal with there is not only 1 arg 
            sprintf(s, "lw $a0, %d($sp)", current2->symbol->offset*WSIZE);// retrieve offset from PTR and store to char s
            emit(fp, "", s, "pull out stored  Arg");// emit to .asm file
            sprintf(s, "move $t%d, $a0", counter);// give a register tx to parameter
            emit(fp, "", s, "move arg in temp ");
            current2 = current2->next;//move to the next node
            counter++;
            if(counter>8)
            { //when there are more than 8 args
                printf("too many arguments we can handle only 8\n");
                exit(1);
            }
        }
        
        fprintf(fp, "\n");// new line after
    }
    sprintf(s, "jal %s", p->name);// store function name to char s
    emit(fp, "", s, "Call the function");//call the function
    fprintf(fp, "\n\n");//space after function call
    counter = 0;//reset the register counter
}// end of emit call

// PRE : Emit break takes break node (break and continue)
// POST : MIPS code to indicate the while loop should stop or continue using j + label
void emit_break(ASTnode *p, FILE * fp)
{
    char * locallabel1;
    char * locallabel2;//local variable to store label
    char s[100];
    if(!isEmpty(GSTACK))
    {   //when stack is not empty
        locallabel1 = top(GSTACK).first;
        locallabel2 = top(GSTACK).second;
        if(p->value == 0)
        {   //it is break stmt
            sprintf(s, "j %s", locallabel2);
            emit(fp, "", s, "");
            emit(fp, "", "", "BREAK Statement line jump inside of while");
        }
        else
        {   // it is continue stmt
            sprintf(s, "j %s", locallabel1);
            emit(fp, "", s, "");
            emit(fp, "", "", "CONTINUE Statement jump inside of while");
        }
         
    }
    else
    {   //when the stack is empty, it should BARF (break and continue cann't be outside of while)
        if(p->value == 0)
        {   //it is break stmt
            printf("ERROR, cannot use BREAK on line %d unless in side a while\n", p->linecount);
            exit(1);//exit the program
        }
        else
        {   // it is continue stmt
            printf("ERROR, cannot use CONTINUE on line %d unless in side a while\n", p->linecount);
            exit(1);//exit the program
        }
    }  

        
}//end of emit_break

//stack implementation to deal with break and continue
// PRE : none
// POST : create a stack structure 
struct Stack* createStack() {
    struct Stack* stack = malloc(sizeof(struct Stack));
    stack->top = -1;
    return stack;
}

//PRE : none
// POST : return true or false depends on the stack is empty or not
int isEmpty(struct Stack* stack) {
    return (stack->top == -1);
}

//PRE : pair parameter
//POST : nothing, but will insert to stack
void push(struct Stack* stack, char* first, char* second) {
    stack->top++;
    stack->pairs[stack->top].first = first;
    stack->pairs[stack->top].second = second;
}

// PRE : take stack name
// POST : remove the top level of stack
void pop(struct Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack underflow\n");
        return;
    }
    stack->top--;
}

// PRE: take stack name
// POST: return top level of the stack
struct Pair top(struct Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty\n");
        struct Pair empty_pair = {NULL, NULL};
        return empty_pair;
    }
    return stack->pairs[stack->top];
}

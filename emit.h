/*
* level f for grading
* emit.h
* Name : Meng-Yang Tseng
* Date : April 11 2023
* interface file for other entities to know about avaiable functions
* provides connectivity to MIPS generating subroutines

* changed :
    * added void EMIT_x(p, fp) function's prototype
    * define WSIZE and LOG_WSIZE
*/

#ifndef EMIT_H
#define EMIT_H
#include "ast.h"
#define WSIZE  4  // number of bytes in MIPS word 
#define LOG_WSIZE 2  // Log base 2 of WSIZE
#define MAX_SIZE 100 // max size of pair stack

void EMIT(ASTnode * p, FILE * fp);
void EMIT_STRINGS(ASTnode * p, FILE * fp);// add for write string 
char * CreateLabel(); //create label
void EMIT_GLOBALS(ASTnode * p, FILE * fp);// for global variable
void emit(FILE *fp, char* label, char* commend, char* comment); //print formated text
void EMIT_AST(ASTnode * p, FILE * fp); //print MIPS code
void emit_function(ASTnode *p, FILE * fp); //print fucntion declaration
void emit_write(ASTnode *p, FILE *fp); //print a_write
void emit_read(ASTnode *p, FILE * fp); //print read
void emit_var(ASTnode *p, FILE * fp); // print var
void emit_return(ASTnode *p, FILE * fp); // print return
void emit_assign(ASTnode *p, FILE * fp);// print assignment
void emit_param(ASTnode *p, FILE * fp);// print params
void emit_selet(ASTnode *p, FILE * fp);// print if else
void emit_while(ASTnode *p, FILE * fp);// print while
void emit_exprstmt(ASTnode *p, FILE * fp);// print expr stmt
void emit_call(ASTnode *p, FILE * fp); // print call stmt
void emit_break(ASTnode *p, FILE * fp); // print break stmt

// implementation of pair stack for break and continue
struct Pair {
    char* first;
    char* second;
};//pair stuct

struct Stack {
    int top;
    struct Pair pairs[MAX_SIZE];
};//stack sturct

struct Stack* createStack();// create the stack
int isEmpty(struct Stack* stack);//check the stack is empty or not
void push(struct Stack* stack, char * first, char * second);//push(insert) the element of stack
void pop(struct Stack* stack);//pop(remove) the element of stack
struct Pair top(struct Stack* stack);//top(get) the element of  stack
#endif
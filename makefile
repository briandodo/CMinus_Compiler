# Name: Meng-Yang Tseng
# Date: April 10 2023
# lab: lab9

#purpose: To compile lab7's files

# change: added ast.c ast.h into dependency and also compile them together
# change: added symtable.h symtable.c dependency and also compile together
# change(lab9): added emit.c, emit.h



all:	lab9 

lab9:	lab9.l lab9.y ast.c ast.h symtable.h symtable.c emit.h emit.c
		lex lab9.l
		yacc -d lab9.y 
		gcc y.tab.c lex.yy.c ast.c symtable.c emit.c -o lab9
		
clean: 	
		rm -f lab9 y.tab.c y.tab.h lex.yy.c
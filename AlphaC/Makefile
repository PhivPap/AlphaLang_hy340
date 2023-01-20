INCLUDE = ADTs/symtable.h ADTs/scopelist.h ADTs/quadarray.h ADTs/logiclist.h ADTs/opt.h ADTs/definitions.h ADTs/v_definitions.h ADTs/instrarray.h ADTs/carray.h parser.h
CSOURCE = ADTs/symtablehash.c ADTs/scopelist.c ADTs/quadarray.c ADTs/logiclist.c ADTs/opt.c ADTs/utility.c ADTs/instrarray.c ADTs/carray.c parser.c scanner.c
OBJS = $(CSOURCE:.c=.o)
RMFILES = parser.c parser.output parser.h scanner.c ex quads.txt
CFLAGS = -std=gnu11 -D __QSTDOUT -D __OPT -O3 #-D __DEBUG
CC = gcc


alphac: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(RMFILES) *.o
	$(RM) ADTs/*.o
	$(RM) alphac

clear:
	clear
	clear

scanner.c: scanner.l
	flex -o$@ $^

scanner.o: scanner.c parser.h
	$(CC) $(CFLAGS) -o $@ -c $<

parser.c parser.h: parser.y
	bison -v --yacc --defines -o parser.c $^

parser.o: parser.c
	$(CC) $(CFLAGS) -o $@ -c $^

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) -c -o $@ $<

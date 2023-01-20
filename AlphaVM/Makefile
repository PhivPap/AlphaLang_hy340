INCLUDE = vstack.h functions.h libfunc.h alphatables.h
CSOURCE = vstack.c alpha.c farithmetics.c frelationals.c ffunctions.c ftables.c libfunc.c alphatables.c
OBJS = $(CSOURCE:.c=.o)
CFLAGS = -std=gnu11 -O3 -lm #-D __DEBUG
CC = gcc

alpha: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(INCLUDE)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(RM) *.o
	$(RM) alpha
	$(RM) out.abc

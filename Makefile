CC      = gcc
CFLAGS  = -Wall

all: calc

calc: lex.yy.c y.tab.c y.tab.h
	$(CC) $(CFLAGS) -o calc lex.yy.c y.tab.c -lfl

y.tab.c y.tab.h: calc.y
	bison -dy calc.y

lex.yy.c: calc.l y.tab.h
	flex calc.l

clean:
	rm -f lex.yy.c y.tab.c y.tab.h calc

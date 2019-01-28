
convertsigs.double:
	gcc -Wall -std=c99 -o convertsigs convertsigs.c

convertsigs.single: convertsigs.c
	gcc -Wall -std=c99 -DSINGLE -o convertsigs convertsigs.c

clean: 	
	rm -f convertsigs

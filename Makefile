
covertsigs.double:
	gcc -g -Wall -std=c99 -o covertsigs covertsigs.c

covertsigs.single: covertsigs.c
	gcc -Wall -std=c99 -DSINGLE -o covertsigs covertsigs.c

clean: 	
	rm -f covertsigs

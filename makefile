CC=gcc
CFLAGS=-g -Wall

test_shell:shell.c manager.c
	$(CC) -o test_shell shell.c manager.c $(CFLAGS)

clean:
	rm -r test_shell

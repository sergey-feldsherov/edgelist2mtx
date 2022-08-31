CFLAGS= -g -O3 -W -Wall -Wunused -Wcast-align -pedantic -pedantic-errors -fstack-protector-all -Wfloat-equal -Wpointer-arith
CC = gcc -std=c11 $(CFLAGS)


all: edglst2mtx

edglst2mtx: src/main.c
	$(CC) src/main.c -o edglst2mtx

clean:
	rm -f edglst2mtx
	find * -name \*~ -delete


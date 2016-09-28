CC=gcc
CFLAGS= -m32
DEPS= memlayout.h 

all: mem_mod1 mem_mod2 mem_mod3

mem_mod1: memlayout.c mem_mod1.c
	gcc -o mem_mod1 memlayout.c mem_mod1.c $(CFLAGS)

mem_mod2: memlayout.c mem_mod2.c
	gcc -o mem_mod2 memlayout.c mem_mod2.c $(CFLAGS)

mem_mod3: memlayout.c mem_mod3.c
	gcc -o mem_mod3 memlayout.c mem_mod3.c $(CFLAGS)
CC=gcc
CFLAGS= -m32
DEPS= memlayout.h 
OBJ = memlayout.c mem_mod1.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mem_mod1: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
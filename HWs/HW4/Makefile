
CC = gcc
CFLAGS = -ansi -Wall -Wshadow -std=gnu99 -lm -lpthread -lrt

makefile: main.c arg.c lib.c list/arraylist.c
	$(CC) -o program main.c arg.c lib.c list/arraylist.c $(CFLAGS)

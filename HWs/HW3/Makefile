
CC = gcc
CFLAGS = -ansi -Wall -Wextra -Wshadow -std=gnu99 -lm -lpthread -lrt

makefile: main.c getopt.c lib.c
	$(CC) -o player main.c getopt.c lib.c $(CFLAGS)

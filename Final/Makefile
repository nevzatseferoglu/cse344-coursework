
CC = gcc
CFLAGS = -ansi -Wall -Wshadow -std=gnu99 -lm -lpthread -lrt

makefile: ./Server/main.c ./Server/arg.c ./Client/main.c ./Client/arg.c ./lib/lib.c ./Server/queue/queue.c
	$(CC) -o server ./Server/main.c ./Server/arg.c ./Server/queue/queue.c ./lib/lib.c $(CFLAGS)
	$(CC) -o client ./Client/main.c ./Client/arg.c ./lib/lib.c $(CFLAGS)

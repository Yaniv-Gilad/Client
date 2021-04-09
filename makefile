all: client.c
	gcc client.c -o client
all-GDB: client.c
	gcc -g client.c -o client

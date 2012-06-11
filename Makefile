install: main.c
	gcc -o ircbot main.c lib/libircclient/libircclient.o -lsqlite3

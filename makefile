CC=gcc
CFLAGS=-O3 -lm 

default: pi_spigot.c
	$(CC) -o pi_spigot pi_spigot.c $(CFLAGS)


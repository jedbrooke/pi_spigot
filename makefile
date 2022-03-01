CC=gcc
CFLAGS=-O3 -lm -g

default: pi_spigot

pi_spigot: pi_spigot.c utility.o fractional64bit.o fixed128.o
	$(CC) -o pi_spigot pi_spigot.c *.o $(CFLAGS)

.PHONY: check
check: pi_spigot check.sh
	./check.sh
	
%.o: %.c
	$(CC) -c $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf pi_spigot *.o
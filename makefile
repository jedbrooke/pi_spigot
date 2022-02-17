CC=gcc
CFLAGS=-O3 -lm 

pi_spigot: pi_spigot.c
	$(CC) -o pi_spigot pi_spigot.c $(CFLAGS)


check: pi_spigot check.sh
	./check.sh
	

.PHONY: clean
clean:
	rm -rf pi_spigot
CC=g++
CFLAGS=-O3 -lm -g --std=c++11

default: pi_spigot

pi_spigot: pi_spigot.cpp 
	$(CC) -o pi_spigot pi_spigot.cpp -I./fractionalBignum $(CFLAGS)

.PHONY: check
check: pi_spigot check.sh
	./check.sh
	
%.o: %.c
	$(CC) -c $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf pi_spigot *.o
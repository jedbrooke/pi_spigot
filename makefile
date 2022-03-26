CC=g++
CFLAGS=-I./fractionalBignum -lm -g --std=c++11 -O3

default: pi_spigot

.PHONY: deps
deps: fractionalBignum.a

fractionalBignum.a: fractionalBignum
	cd fractionalBignum; \
	make; \
	cp fractionalBignum.a ..


pi_spigot: pi_spigot.cpp deps
	$(CC) -o pi_spigot pi_spigot.cpp fractionalBignum.a $(CFLAGS)

.PHONY: check
check: pi_spigot check.sh
	./check.sh
	
%.o: %.c
	$(CC) -c $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf pi_spigot *.o *.a
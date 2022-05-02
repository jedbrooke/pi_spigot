CC=g++
CFLAGS=-I./fractionalBignum --std=c++14 -O3
LDFLAGS=-lm -pthread

default: pi_spigot


fractionalBignum.a: fractionalBignum
	cd fractionalBignum; \
	make; \
	cp fractionalBignum.a ..


pi_spigot: pi_spigot.cpp fractionalBignum.a WorkManager.o Worker.o modpow.o
	$(CC) $(CFLAGS) -o pi_spigot pi_spigot.cpp *.o fractionalBignum.a $(LDFLAGS) 

.PHONY: check
check: pi_spigot check.sh
	./check.sh

%.o: %.cpp
	$(CC) -c $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf pi_spigot *.o *.a
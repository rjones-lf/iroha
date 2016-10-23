all: libhash.a

CC = g++
KECCAK_PATH = ../../vendor/KeccakCodePackage

DEFINE  = -DKeccakP200_excluded -DKeccakP400_excluded -DKeccakP800_excluded

CFLAGS := $(CFLAGS) -std=c++0x
CFLAGS := $(CFLAGS) $(DEFINE)
CFLAGS := $(CFLAGS) -I$(KECCAK_PATH)/bin/generic64/libkeccak.a.headers

KeccakP-1600-opt64.o: $(KECCAK_PATH)/SnP/KeccakP-1600/Optimized64/KeccakP-1600-opt64.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
OBJECTS := $(OBJECTS) KeccakP-1600-opt64.o

KeccakSponge.o: $(KECCAK_PATH)/Constructions/KeccakSponge.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
OBJECTS := $(OBJECTS) KeccakSponge.o

SimpleFIPS202.o: $(KECCAK_PATH)/Modes/SimpleFIPS202.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
OBJECTS := $(OBJECTS) SimpleFIPS202.o

libhash.a: $(OBJECTS)
	$(CC) $(CFLAGS) hash.cpp -c
	ar rcs libhash.a hash.o $(OBJECTS)

clean:
	rm -rf libhash.a *.o

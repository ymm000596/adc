CC=gcc
LDFLAGS+=-lasound -lrt -lzmq

noise.exe:main.o ltpro.o
	$(CC) main.o ltpro.o $(LDFLAGS) -o adc.exe

main.o:main.c
	$(CC) main.c -c -O3 -o main.o

ltpro.o:ltpro.c
	$(CC) ltpro.c -c -O3 -o ltpro.o

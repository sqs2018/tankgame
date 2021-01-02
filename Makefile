#MAIN    : main
CC = gcc
CONFIG += console

INCLUDEPATH +=-L/usr/include/SDL

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx

main : main.o
	cc -o main main.o -lSDL -lSDL_image  -Wimplicit-function-declaration
main.o: main.c
	cc -c main.c 
clean :
	rm -f *.o


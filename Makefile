GCC=gcc
GCC_FLAGS=-Wall
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
PROGRAM=src

all:
	gcc -Wall src/master.c -o master
	gcc -Wall src/slave.c -o slave

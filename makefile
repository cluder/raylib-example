
override CFLAGS += -Wall -Wextra -c -g

#SRCS = $(wildcard *.c)
#OBJS = $(SRCS:.c=.o)

LDLIBS=-lraylib
#EXENAME=raylib_example
#EXEDIR=.
#EXEPATH=$(EXEDIR)/$(EXENAME)

#all: $(EXEPATH)

raylib_example: raylib_example.o
	gcc -g  -o raylib_example raylib_example.o $(LDLIBS)


raylib_example.o: raylib_example.c 
	gcc $(CFLAGS) raylib_example.c

clean:
	rm raylib_example raylib_example.o

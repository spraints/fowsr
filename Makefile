CC = gcc
#CFLAGS += -std=c99
NAME = fowsr
SRC = fowsr.c
LDFLAGS += -lm -lusb -ljson-c

all:
	$(CC) $(CFLAGS) -o $(NAME) $(SRC) $(LDFLAGS) -g

clean:
	-rm -f $(NAME) *.d *.o

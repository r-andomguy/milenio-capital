CC = gcc
CFLAGS = -fPIC -Wall -Wextra -Iinclude
LDFLAGS = -shared

LIB_NAME = libcsv.h
SRC = app/libcsv.c
OBJ = $(SRC:.c=.o)

all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB_NAME)

.PHONY: all clean


CC = gcc
CFLAGS = -g -Wall
LIBS = -lreadline
SRC = src/main.c src/sh.c src/trie.c
OUT = fynsh

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT) a.out

run: $(OUT)
	./$(OUT)
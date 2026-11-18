CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -lm
BIN = bin/shae-fortran-matrix-calculator

.PHONY: all clean install

all: $(BIN)

$(BIN): src/main.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ src/main.c

clean:
	rm -rf bin

install: $(BIN)
	mkdir -p ~/.local/bin
	cp $(BIN) ~/.local/bin/

release: $(BIN)
	gh release create "v$(shell date +%Y%m%d%H%M%S)" $(BIN) --generate-notes
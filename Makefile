CFLAGS=-std=c11 -O2 -g -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)
LIBS=`pkg-config --libs openssl` $(OPTLIBS)

PREFIX?=/usr/local

SOURCES=$(wildcard src/*.c src/**/*.c)
OBJECTS=$(patsubst src/%.c,objs/%.o,$(SOURCES))
HEADERS=$(wildcard src/*.h src/**/*.h)

TARGET=build/web-watch

all: $(TARGET) $(TEST_TARGETS)

dev: CFLAGS=-g -Wall -Wextra -Isrc $(OPTFLAGS)
dev: all

$(TARGET): build $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS) $(LIBS)


objs/%.o: src/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

build:
	@mkdir -p build
	@mkdir -p objs

.PHONY: clean
clean:
	rm -rf build/*
	rm -rf objs/*

install:
	@true


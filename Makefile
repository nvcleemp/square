
SOURCES = square.c\
          Makefile COPYRIGHT.txt LICENSE.txt README.md

all: build/square

clean:
	rm -rf build
	rm -rf dist

build/square: square.c
	mkdir -p build
	cc -o $@ -O4 -Wall $^

sources: dist/square-sources.zip dist/square-sources.tar.gz

dist/square-sources.zip: $(SOURCES)
	mkdir -p dist
	zip dist/square-sources $(SOURCES)

dist/square-sources.tar.gz: $(SOURCES)
	mkdir -p dist
	tar czf dist/square-sources.tar.gz $(SOURCES)

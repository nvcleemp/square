
SOURCES = st.c\
          Makefile COPYRIGHT.txt LICENSE.txt README.md

all: build/st

clean:
	rm -rf build
	rm -rf dist

build/st: st.c
	mkdir -p build
	cc -o $@ -O4 -Wall $^

sources: dist/st-sources.zip dist/st-sources.tar.gz

dist/st-sources.zip: $(SOURCES)
	mkdir -p dist
	zip dist/st-sources $(SOURCES)

dist/st-sources.tar.gz: $(SOURCES)
	mkdir -p dist
	tar czf dist/st-sources.tar.gz $(SOURCES)

all: build
.PHONY: all

build:
	gcc -o mongowatch base64.c mongowatch.c -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0 -L/usr/local/lib -lmongoc-1.0 -lssl -lcrypto -lrt -lresolv -lz -lbson-1.0
.PHONY: build

CFLAGS = -march=native -pipe -fomit-frame-pointer -flto -O3

all: test benchmark
test: domain-lookup.c domain-lookup.h test.c
benchmark: domain-lookup.c domain-lookup.h benchmark.c
clean:
	rm -f test benchmark

.PHONY: clean

CC = g++
CFLAGS = -O3 -std=c++11 -I../ -I../include
DEFS = -DLINUX
LDFLAGS = -Llib -Wl,-rpath=lib
LDLIBS = -lrt -ltactraderapi

all: trade_demo

trade_demo: trade_demo.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f trade_demo

.PHONY: all trade_demo clean

CC := gcc
CXX := g++
.DEFAULT_GOAL := all
DEST := .

module_list := wftd-tac.so
wftd-tac.so: trader_tac.o
	$(CXX) -shared -o $@ $^ -lrt -ltactraderapi

########## for frontend dev END #############################

CFLAGS := -Wall -I $(DEST)/include -I ${HOME}/.local/include -fPIC -O3 -g -pthread
CXXFLAGS := $(CFLAGS) --std=c++11

all: $(module_list)

install: $(module_list)
	install -d $(DEST)/lib
	install -p $(module_list) $(DEST)/lib

clean:
	$(RM) $(module_list) *.o

.PHONY: all install clean

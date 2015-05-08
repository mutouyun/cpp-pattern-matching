# Compiler, tools and options

CC = gcc
CX = g++
LB = ar -csr
DEFINES =
CFLAGS = -pipe -frtti -Wall -Wextra -fexceptions -march=nocona -c -std=c++1y
LFLAGS = -Wl,-s
INCPATH = -I"./"

CFLAGS += -O2 -DNDEBUG
Configuration = release

# Output directory

OUT = ./build/bin/$(Configuration)/$(CC)
TMP = ./build/tmp/$(Configuration)/$(CC)

# Build rules

.PHONY: all clean out tmp

all: $(TMP)/match_gcc/main.o match_gcc

clean:
	-rm -fr ./build

out:
	-mkdir -p $(OUT)

tmp:
	-mkdir -p $(TMP)

# Compile

$(TMP)/match_gcc/main.o: ./main.cpp | tmp
	$(CX) -o $(TMP)/main.o $(CFLAGS) $(INCPATH) ./main.cpp

match_gcc: $(TMP)/main.o | out
	$(CX) -o $(OUT)/match $(LFLAGS) $(TMP)/main.o


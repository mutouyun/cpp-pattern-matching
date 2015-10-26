# Compiler, tools and options

CC ?= gcc
CX ?= g++
LB ?= ar -csr
DEFINES ?=
CFLAGS ?= -pipe -frtti -Wall -Wextra -fexceptions -march=nocona -c -std=c++1y
LFLAGS ?= -Wl,-s
INCPATH ?= -I"./"

debug = 0
ifeq ($(debug), 0)
	# release
	CFLAGS += -O2 -DNDEBUG
	CONFIG_DIR = release
else
	# debug
	CFLAGS += -g
	CONFIG_DIR = debug
endif

# Output directory

OUT = ./build/bin/$(CONFIG_DIR)/$(CC)
TMP = ./build/tmp/$(CONFIG_DIR)/$(CC)

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


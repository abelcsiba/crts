
CC=gcc
CFLAGS= -fPIC -Iinclude/ -I../common/include/ -I../ciam/include/
WFLAGS=-Wall -Wextra

TARGET=core

.PHONY: all clean io

all: $(TARGET)

$(TARGET): io
	@echo "Compiling core modules"

io:
	@echo "Compiling io module"
	$(MAKE) -C io all
	@echo "Compiled io module"

clean:
	$(MAKE) -C io clean
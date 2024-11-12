
CC=cc
CFLAGS= -fPIC -Iinclude/ -I../common/include/ -I../ciam/include/
WFLAGS=-Wall -Wextra

MODULE_DIRS=io
TARGET=core

.PHONY: all clean io

all: $(TARGET)

$(TARGET): $(MODULE_DIRS)
	@echo "Compiling core modules"

$(MODULE_DIRS):
	@echo "Compiling io module"
	$(MAKE) -C $@ all
	@echo "Compiled io module"

clean:
	@for mod_dir in $(MODULE_DIRS); do 		\
		$(MAKE) -C $$mod_dir clean;			\
	done


CC=gcc
CFLAGS=-Wall -Wextra -MMD -MP
INCLUDES=-Iciam/include -Icias/include -Icommon/include
LDFLAGS=-Lciam/lib/ -Lcias/lib/ -Lcommon/lib/

LIBS=-lciam -lcias -lcommon -lm

SRC=main.c

OBJ_DIR=obj

OBJ=$(SRC:%.c=$(OBJ_DIR)/%.o)

EXEC=oizys
CORE=core

CIAM_SRC=$(wildcard ciam/src/*.c)
CIAS_SRC=$(wildcard cias/src/*.c)
COMMON_SRC=$(wildcard common/src/*.c)

CORE_IO_SRC=$(wildcard core/io/src/*.c)

DEPS=$(OBJ:.o=.d)

.PHONY: all clean

all: ciam/lib/libciam.so cias/lib/libcias.so common/lib/libcommon.so $(CORE) $(EXEC)

ciam/lib/libciam.so: $(CIAM_SRC)
	$(MAKE) -C ciam

cias/lib/libcias.so: $(CIAS_SRC)
	$(MAKE) -C cias

common/lib/libcommon.so: $(COMMON_SRC)
	$(MAKE) -C common

$(CORE): $(CORE_IO_SRC)
	$(MAKE) -C core

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(INCLUDES) $(LIBS) -o $@

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -g

clean:
	$(MAKE) -C ciam clean
	$(MAKE) -C cias clean
	$(MAKE) -C common clean
	$(MAKE) -C core clean
	rm -f $(OBJ) $(EXEC)
	rm -rf $(OBJ_DIR)

-include $(DEPS)

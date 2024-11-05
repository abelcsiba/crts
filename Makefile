

CC=gcc
CFLAGS=-Wall -Wextra
INCLUDES=-Iciam/include -Icias/include -Icommon/include
LDFLAGS=-Lciam/lib/ -Lcias/lib/ -Lcommon/lib/

LIBS=-lciam -lcias -lcommon -lm

SRC=main.c

OBJ_DIR=obj

OBJ=$(SRC:%.c=$(OBJ_DIR)/%.o)

EXEC=oizys

.PHONY: all clean

all: ciam/lib/libciam.so cias/lib/libcias.so common/lib/libcommon.so $(EXEC)

ciam/lib/libciam.so:
	$(MAKE) -C ciam

cias/lib/libcias.so:
	$(MAKE) -C cias

common/lib/libcommon.so:
	$(MAKE) -C common

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(INCLUDES) $(LIBS) -o $@

$(OBJ): *.c
	mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -g

clean:
	$(MAKE) -C ciam clean
	$(MAKE) -C cias clean
	$(MAKE) -C common clean
	rm -f $(OBJ) $(EXEC)
	rm -rf $(OBJ_DIR)




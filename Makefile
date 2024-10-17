

CC=gcc
CFLAGS=-Wall -Wextra
INCLUDES=-Iciam/include -Icias/include -Icommon
LDFLAGS=-Lciam/lib/ -Lcias/lib/

LIBS=-lciam -lcias -lm

SRC=main.c

OBJ_DIR=obj

OBJ=$(SRC:%.c=$(OBJ_DIR)/%.o)

EXEC=irtn

.PHONY: all clean

all: ciam/lib/libciam.so cias/lib/libcias.so $(EXEC)

ciam/lib/libciam.so:
	$(MAKE) -C ciam

cias/lib/libcias.so:
	$(MAKE) -C cias

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(INCLUDES) $(LIBS) -o $@

$(OBJ): *.c
	mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -g

clean:
	$(MAKE) -C ciam clean
	$(MAKE) -C cias clean
	rm -f $(OBJ) $(EXEC)
	rm -rf $(OBJ_DIR)




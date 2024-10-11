

CC=gcc
CFLAGS=-Wall -Wextra
INCLUDES=-Iciam/include 
LDFLAGS=-Lciam/lib/ 

LIBS=-lciam

SRC=main.c

OBJ_DIR=obj

OBJ=$(SRC:%.c=$(OBJ_DIR)/%.o)

EXEC=irtn

.PHONY: all clean

all: ciam/lib/libciam.so $(EXEC)

ciam/lib/libciam.so:
	$(MAKE) -C ciam

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(INCLUDES) $(LIBS) -o $@

$(OBJ): *.c 
	mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -g

clean:
	$(MAKE) -C ciam clean
	rm -f $(OBJ) $(EXEC)
	rm -rf $(OBJ_DIR)




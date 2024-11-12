
CC=cc
CFLAGS= -fPIC -Iinclude/ -I../common/include/
WFLAGS=-Wall -Wextra

LDFLAGS=-shared

SRC_DIR=src
OBJ_DIR=obj
LIB_DIR=lib

SOURCE_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJECT_FILES=$(SOURCE_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

RM=rm

TARGET=$(LIB_DIR)/libcdb.so

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECT_FILES) | $(LIB_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@ -g

$(LIB_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rfv $(LIB_DIR) $(OBJ_DIR)
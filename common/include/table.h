
#ifndef COMMON_TABLE_H_
#define COMMON_TABLE_H_

#include "value.h"

#include <stdbool.h>

// HashTable implementation stolen from https://github.com/munificent/craftinginterpreters
typedef struct {
    obj_string_t*   key;
    value_t         value;
} entry_t;

typedef struct {
    entry_t*        entries;
    int             size;
    int             capacity;
} table_t;

void table_init(table_t* table);
void table_free(table_t* table);

bool table_get(table_t* table, obj_string_t* key, value_t* value);
bool table_set(table_t* table, obj_string_t* key, value_t value);
bool table_delete(table_t* table, obj_string_t* key);


#endif // COMMON_TABLE_H_
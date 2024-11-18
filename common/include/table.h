
#ifndef COMMON_TABLE_H_
#define COMMON_TABLE_H_

#include "value.h"

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


#endif // COMMON_TABLE_H_
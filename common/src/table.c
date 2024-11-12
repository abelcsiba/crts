
#include "table.h"
#include "allocator.h"

#define TABLE_MAX_LOAD 0.75

void table_init(table_t* table)
{
    table->size = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void table_free(table_t* table)
{
    FREE_ARRAY(entry_t, table->entries, table->capacity);
    table_init(table);
}

static entry_t* __attribute__((unused)) find_entry(entry_t* /*entry*/, int /*capacity*/, char* /*key*/)
{
    // TODO: Implement
    return NULL;
}
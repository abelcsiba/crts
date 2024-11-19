
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

static entry_t* find_entry(entry_t* entries, int capacity, obj_string_t* key)
{
    uint32_t index = key->hash & (capacity - 1);
    entry_t* tombstone = NULL;

    for (;;) 
    {
        entry_t* entry = &entries[index];

        if (entry->key == NULL) 
        {
            if      ( IS_NULL(entry->value )) return tombstone != NULL ? tombstone : entry;
            else if ( tombstone == NULL     ) tombstone = entry;
            else if ( entry->key == key     ) return entry;

            index = (index + 1) & (capacity - 1);
        }
    }
}

bool table_get(table_t* table, obj_string_t* key, value_t* value)
{
    if (table->size == 0) return false;

    entry_t* entry = find_entry(table->entries, table->capacity, key);

    if (NULL == entry) return false;
    *value = entry->value;
    return true;
}

static void adjust_capacity(table_t* table, int capacity)
{
    entry_t* entries = ALLOCATE(entry_t, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NULL_VAL(0x00);
    }

    table->size = 0;

    for (int i = 0; i < table->capacity; i++) {
        entry_t* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        entry_t* dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->size++;
    }

    FREE_ARRAY(entry_t, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool table_set(table_t* table, obj_string_t* key, value_t value)
{
    if (table->size + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjust_capacity(table, capacity);
    }

    entry_t* entry = find_entry(table->entries, table->capacity, key);
    bool is_new_key = entry->key == NULL;

    if (is_new_key && IS_NULL(entry->value)) table->size++;

    entry->key = key;
    entry->value = value;
    return is_new_key;
}

bool table_delete(table_t* table, obj_string_t* key)
{
    if (table->size == 0) return false;

    entry_t* entry = find_entry(table->entries, table->capacity, key);
    if (NULL == entry) return false;

    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}
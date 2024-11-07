
#include "native.h"
#include "helpers.h"

#include <string.h>
#include <stdio.h>

void print(struct ciam_vm_t* /*vm*/, value_t* values, size_t argc)
{
    for (size_t i = 0; i < argc; i++)
    {
        if (i != 0) printf(" ");
        print_value(values[i]);
    }
    printf("\n");
}

#define NATIVE_LIST                       \
    X(PRINT, "print", print)          \

const native_entry_t native_names[] = {
#define X(ID, LABEL, PTR) { .native_name = LABEL, .native = PTR},
    NATIVE_LIST
#undef X
};

const size_t native_count = sizeof(native_names) / sizeof(native_names[0]);

native_ptr_t get_native(const char* name)
{
    for (size_t i = 0; i < native_count; i++)
        if (strcmp(name, native_names[i].native_name) == 0)
            return native_names[i].native;
    return NULL;
}

#undef NATIVE_LIST
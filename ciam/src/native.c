
#include "native.h"
#include "helpers.h"

#include <string.h>
#include <stdio.h>

value_t print(struct ciam_vm_t* /*vm*/, value_t* values, size_t argc)
{
    for (size_t i = 0; i < argc; i++)
    {
        if (i != 0) printf(" ");
        print_value(values[i]);
    }
    printf("\n");
    return (value_t){ .type = VAL_VOID, .as.i8 = 0x00 };
}

value_t read(struct ciam_vm_t* vm, value_t* values, size_t argc)
{
    char *buffer = NULL;
    size_t bufsize = 0;
    print(vm, values, argc);

    int len = getline(&buffer, &bufsize, stdin);
    if (len != -1) {
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len -= 1;
        }
    } else {
        fprintf(stderr, "Error reading input\n");
        exit(EXIT_FAILURE);
    }
    obj_string_t* obj = (obj_string_t*)calloc(1, sizeof(obj_string_t));
    obj->chars = buffer;
    obj->length = len;

    return OBJ_VAL((obj_t*)obj);
}

#define NATIVE_LIST                       \
    X(PRINT,    "print",    print   )     \
    X(READ,     "read",     read    )     \

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
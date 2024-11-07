
#ifndef CIAM_NATIVE_H_
#define CIAM_NATIVE_H_

#include "value.h"

#include <stdlib.h>

struct ciam_vm_t;
typedef void (*native_ptr_t)(struct ciam_vm_t*, value_t*, size_t);    

typedef struct {
    char*           native_name;
    native_ptr_t    native;
} native_entry_t;

void print(struct ciam_vm_t* vm, value_t* value, size_t argc);

native_ptr_t get_native(const char* name);


#endif // CIAM_NATIVE_H_
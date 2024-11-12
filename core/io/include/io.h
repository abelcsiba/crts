
#ifndef CORE_IO_H_
#define CORE_IO_H_

#include "value.h"
#include <stdlib.h>

struct ciam_vm_t;
value_t max(struct ciam_vm_t* vm, value_t* values, size_t argc);

#endif // CORE_IO_H_
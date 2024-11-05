

#include "helpers.h"

#include <stdio.h>
#include <inttypes.h>

void print_values(value_t val) {
    switch (val.type) 
    {
        case VAL_I8: // Assuming all integer types use %d
            printf("%d", val.as.i8); // For simplicity, using i32 for demo
            break;
        case VAL_I16:
            printf("%d", val.as.i16);
            break;
        case VAL_I32:
            printf("%d", val.as.i32);
            break;
        case VAL_I64:
            printf("%ld", val.as.i64);
            break;
        case VAL_FLOAT:
            printf("%f", val.as.flt);
            break;
        case VAL_BOOL:
            printf("%s", val.as.boolean ? "true" : "false");
            break;
        case VAL_DOUBLE:
            printf("%lf", val.as.dbl);
            break;
        case VAL_CHAR:
            printf("%c", val.as.chr);
            break;
        default:
            printf("Unsupported format type");
            break;
    }
}
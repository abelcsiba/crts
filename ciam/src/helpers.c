

#include "helpers.h"

#include <stdio.h>
#include <inttypes.h>

void print_value(value_t val) {
    switch (val.type) 
    {
        case VAL_I8:
            printf("%d", val.as.i8);
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
        case VAL_OBJECT:
            if (OBJ_STRING != val.as.obj->obj_type) printf("OBJ"); // TODO: Support other obj types
            obj_string_t* str = (obj_string_t*)val.as.obj;
            printf("%.*s", (int)str->length, str->chars);
            break;
        default:
            printf("Unsupported format type");
            break;
    }
}
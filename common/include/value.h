
#ifndef CIAM_VALUE_H_
#define CIAM_VALUE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define VALUE_TYPE_LIST             \
    X(VAL_I8,       "I8"        )   \
    X(VAL_I16,      "I16"       )   \
    X(VAL_I32,      "I32"       )   \
    X(VAL_I64,      "I64"       )   \
    X(VAL_FLOAT,    "FLOAT"     )   \
    X(VAL_DOUBLE,   "DOUBLE"    )   \
    X(VAL_BOOL,     "BOOL"      )   \
    X(VAL_CHAR,     "CHAR"      )   \
    X(VAL_OBJECT,   "OBJECT"    )   \
    X(VAL_VOID,     "VOID"      )   \
    X(VAL_NULL,     "NULL"      )   \
    

typedef enum {
    #define X(type, str) type,
    VALUE_TYPE_LIST
    #undef X
} valuetype_t;

#define IS_NULL(value)      ((value).type == VAL_NULL)

#define AS_I8(value)        ((value).as.i8      )
#define AS_I16(value)       ((value).as.i8      )
#define AS_I32(value)       ((value).as.i8      )
#define AS_I64(value)       ((value).as.i8      )
#define AS_FLOAT(value)     ((value).as.flt     )
#define AS_DOUBLE(value)    ((value).as.dbl     )
#define AS_BOOL(value)      ((value).as.boolean )
#define AS_CHAR(value)      ((value).as.chr     )
#define AS_OBJ(value)       ((value).as.obj     )

#define I8_VAL(value)       ((value_t){ VAL_I8,     { .i8       = value }})
#define I16_VAL(value)      ((value_t){ VAL_I16,    { .i16      = value }})
#define I32_VAL(value)      ((value_t){ VAL_I32,    { .i32      = value }})
#define I64_VAL(value)      ((value_t){ VAL_I64,    { .i64      = value }})
#define FLOAT_VAL(value)    ((value_t){ VAL_FLOAT,  { .flt      = value }})
#define DOUBLE_VAL(value)   ((value_t){ VAL_DOUBLE, { .dbl      = value }})
#define BOOL_VAL(value)     ((value_t){ VAL_BOOL,   { .boolean  = value }})
#define CHAR_VAL(value)     ((value_t){ VAL_CHAR,   { .chr      = value }})
#define OBJ_VAL(value)      ((value_t){ VAL_OBJECT, { .obj      = value }})
#define NULL_VAL(value)     ((value_t){ VAL_NULL,   { .i8       = value }})

#define OBJ_TYPE(value)     (AS_OBJ(value)->obj_type)

typedef struct obj_t obj_t;
typedef struct obj_string_t obj_string_t;

typedef enum {
    OBJ_STRING
} obj_type_t;

struct obj_t {
    obj_type_t          obj_type;
    bool                marked;
    obj_t*              next;
};

struct obj_string_t {
    obj_t               obj;
    uint32_t            length;
    char*               chars;
    uint32_t            hash;
};

typedef struct {
    valuetype_t         type;

    union {
        int8_t          i8;
        int16_t         i16;
        int32_t         i32;
        int64_t         i64;
        float           flt;
        double          dbl;
        bool            boolean;
        char            chr;
        obj_t*          obj;
    } as;
} value_t;

#endif // CIAM_VALUE_H_

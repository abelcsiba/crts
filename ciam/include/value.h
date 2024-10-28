
#ifndef CIAM_VALUE_H_
#define CIAM_VALUE_H_

#include <stdbool.h>
#include <stdint.h>

#define VALUE_TYPE_LIST         \
    X(VAL_I8,       "I8")       \
    X(VAL_I16,      "I16")      \
    X(VAL_I32,      "I32")      \
    X(VAL_I64,      "I64")      \
    X(VAL_FLOAT,    "FLOAT")    \
    X(VAL_DOUBLE,   "DOUBLE")   \
    X(VAL_BOOL,     "BOOL")     \
    X(VAL_CHAR,     "CHAR")     \
    X(VAL_OBJECT,   "OBJECT")   \

typedef enum {
    #define X(type, str) type,
    VALUE_TYPE_LIST
    #undef X
} valuetype_t;

#define AS_I8(value)        ((value).as.i8)
#define AS_I16(value)       ((value).as.i8)
#define AS_I32(value)       ((value).as.i8)
#define AS_I64(value)       ((value).as.i8)
#define AS_FLOAT(value)     ((value).as.flt)
#define AS_DOUBLE(value)    ((value).as.dbl)
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_CHAR(value)      ((value).as.chr)

#define I8_VAL(value)       ((value_t){ VAL_I8,     { .i8 = value }})
#define I16_VAL(value)      ((value_t){ VAL_I16,    { .i16 = value }})
#define I32_VAL(value)      ((value_t){ VAL_I32,    { .i32 = value }})
#define I64_VAL(value)      ((value_t){ VAL_I64,    { .i64 = value }})
#define FLOAT_VAL(value)    ((value_t){ VAL_FLOAT,  { .flt = value }})
#define DOUBLE_VAL(value)   ((value_t){ VAL_DOUBLE, { .dbl = value }})
#define BOOL_VAL(value)     ((value_t){ VAL_BOOL,   { .boolean = value }})
#define CHAR_VAL(value)     ((value_t){ VAL_CHAR,   { .chr = value }})

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
    } as;
} value_t;

#endif // CIAM_VALUE_H_


#ifndef CIAM_COMMON_H_
#define CIAM_COMMON_H_

#include <stdint.h>

typedef int64_t i64;
typedef uint64_t u64;
typedef int8_t byte;

#define DECL_DA(type)                                                                       \
typedef struct {                                                                            \
    type* data;                                                                             \
    size_t count;                                                                           \
    size_t capacity;                                                                        \
} type##_array_t;                                                                           \
                                                                                            \
void add_##type(type##_array_t* da, type val);                                              \
void init_##type##_array_t(type##_array_t* da);                                             \


#define DEF_DA(type)                                                                        \
void add_##type(type##_array_t* da, type val)                                               \
{                                                                                           \
    if (da->count == da->capacity)                                                          \
    {                                                                                       \
        size_t new_cap = (0 == da->capacity ? 8 : da->capacity * 2);                        \
        da->data = (type*)realloc(da->data, sizeof(type) * new_cap);                        \
        da->capacity = new_cap;                                                             \
    }                                                                                       \
    da->data[da->count++] = val;                                                            \
}                                                                                           \
                                                                                            \
void init_##type##_array_t(type##_array_t* da)                                              \
{                                                                                           \
    da->count = da->capacity = 0;                                                           \
    da->data = NULL;                                                                        \
}                                                                                           \


#define CIAM_MAGIC_NUM 0x6c6562c1
#define CIAM_VER_MAJOR 0x0
#define CIAM_VER_MINOR 0x1
#define CIAM_VER_PATCH 0x0

#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))

PACK(struct ciam_header_t {
    uint32_t magic;
    uint32_t version;
    uint64_t code_size;
    uint64_t const_size;
    uint64_t main_addr;
});

typedef struct ciam_header_t ciam_header_t;

#endif // CIAM_COMMON_H_

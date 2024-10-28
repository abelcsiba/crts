
#ifndef COMMON_CODEC_H_
#define COMMON_CODEC_H_

#include "memory.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define CIAM_MAGIC_NUM      0x6c6562c1
#define CIAM_VER_MAJOR      0x00
#define CIAM_VER_MINOR      0x01
#define CIAM_VER_PATCH      0x00
#define CIAM_VER_REV        0x00

#define ENCODE_CIAM_VERSION(version) (version | ((uint32_t)CIAM_VER_MAJOR << 3 * 8) |   \
                                                ((uint32_t)CIAM_VER_MINOR << 2 * 8) |   \
                                                ((uint32_t)CIAM_VER_PATCH << 1 * 8) |   \
                                                ((uint32_t)CIAM_VER_REV   << 0 * 8))    \


#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))

PACK(struct time_stamp_t {
    uint32_t        sec;
    uint32_t        min;
    uint32_t        hour;
    uint32_t        m_day;
    uint32_t        mon;
    uint32_t        year;
    uint32_t        w_day;
    uint32_t        y_day;
    uint32_t        is_dst;
});

typedef struct time_stamp_t ts_t;

PACK(struct ciam_header_t {
    uint32_t        magic;
    uint32_t        version;
    ts_t            timestamp;
    uint64_t        code_size;   // Maybe remove..?
    uint64_t        const_size;  // Maybe remove..?
    uint64_t        main_addr;
});

typedef struct ciam_header_t ciam_header_t;

#define NUM_CONST_SIZE 9

void    encode(char* buff, module_t* module);
void    decode(char* buff, module_t* module);

#endif // COMMON_CODEC_H_
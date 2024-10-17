
#ifndef COMMON_CODEC_H_
#define COMMON_CODEC_H_

#include "../ciam/include/code.h"

#include <stdint.h>
#include <string.h>

#define CIAM_MAGIC_NUM 0x6c6562c1
#define CIAM_VER_MAJOR 0x00
#define CIAM_VER_MINOR 0x01
#define CIAM_VER_PATCH 0x00
#define CIAM_VER_REV   0x00

#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))

PACK(struct ciam_header_t {
    uint32_t magic;
    uint32_t version;
    uint64_t code_size;
    uint64_t const_size;
    uint64_t main_addr;
});

typedef struct ciam_header_t ciam_header_t;

void encode(char* buff, code_t *code)
{
    ciam_header_t header;
    header.magic = CIAM_MAGIC_NUM;
    int32_t version = 0;
    version |= ((uint32_t)CIAM_VER_MAJOR << 3 * 8);
    version |= ((uint32_t)CIAM_VER_MINOR << 2 * 8);
    version |= ((uint32_t)CIAM_VER_PATCH << 1 * 8);
    version |= ((uint32_t)CIAM_VER_REV   << 0 * 8);
    header.version = version;
    header.code_size = 0xA214;
    header.const_size = 0x9988;
    header.main_addr = sizeof(ciam_header_t);
    memcpy(buff, &header, sizeof(ciam_header_t));
}


#endif // COMMON_CODEC_H_
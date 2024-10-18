
#ifndef COMMON_CODEC_H_
#define COMMON_CODEC_H_

#include "../ciam/include/code.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define CIAM_MAGIC_NUM 0x6c6562c1
#define CIAM_VER_MAJOR 0x00
#define CIAM_VER_MINOR 0x01
#define CIAM_VER_PATCH 0x00
#define CIAM_VER_REV   0x00

#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))

PACK(struct time_stamp_t {
    uint32_t sec;
    uint32_t min;
    uint32_t hour;
    uint32_t m_day;
    uint32_t mon;
    uint32_t year;
    uint32_t w_day;
    uint32_t y_day;
    uint32_t is_dst;
});

typedef struct time_stamp_t ts_t;

PACK(struct ciam_header_t {
    uint32_t magic;
    uint32_t version;
    ts_t timestamp;
    uint64_t code_size;   // Maybe remove..?
    uint64_t const_size;  // Maybe remove..?
    uint64_t main_addr;
});

static int op_has_operand[] = {
#define X(kind, id, has_operand) [id] = has_operand,
    OPCODE_LIST
#undef X
};

typedef struct ciam_header_t ciam_header_t;

#define NUM_CONST_SIZE 9

typedef struct {
    uint8_t type;
    uint64_t value;
} num_const_t;

typedef struct {
    num_const_t* nums;
    uint16_t count;
} const_num_table_t;

typedef struct {
    const_num_table_t numbers;
} const_pool_t;

static size_t encode_numeric_constants(char* buff, const_num_table_t* num_table)
{
#define ENCODE(X) do { memcpy(buff + offset, &X, sizeof(X)); offset += sizeof(X); } while (false)
    size_t offset = 0;
    ENCODE(num_table->count);
    for (uint16_t i = 0; i < num_table->count; i++)
    {
        num_const_t num = num_table->nums[i];
        ENCODE(num.type);
        ENCODE(num.value);
    }
    return offset;
#undef ENCODE
}

static size_t encode_inst_code(char* buff, code_t* code, size_t code_size)
{
#define ENCODE(X) do { memcpy(buff + offset, &X, sizeof(X)); offset += sizeof(X); } while (false)
    size_t offset = 0;
    for (size_t i = 0; i < code_size; i++)
    {
        ENCODE(code[i].op);
        if (op_has_operand[code[i].op] == 1) ENCODE(code[i].opnd1);
        else offset += sizeof(code[i].opnd1);
    }
    return offset;
#undef ENCODE
}

static void encode_timestamp(ts_t* timestamp)
{
    time_t raw_time;
    time(&raw_time);
    struct tm* local_time = localtime(&raw_time);

    timestamp->sec      = local_time->tm_sec;
    timestamp->min      = local_time->tm_min;
    timestamp->hour     = local_time->tm_hour;
    timestamp->m_day    = local_time->tm_mday;
    timestamp->mon      = local_time->tm_mon;
    timestamp->year     = local_time->tm_year;
    timestamp->w_day    = local_time->tm_wday;
    timestamp->y_day    = local_time->tm_yday;
    timestamp->is_dst   = local_time->tm_isdst;
}

void encode(char* buff, code_t* code, size_t code_size, const_pool_t* pool)
{
    ciam_header_t header;
    header.magic = CIAM_MAGIC_NUM;
    int32_t version = 0;
    version |= ((uint32_t)CIAM_VER_MAJOR << 3 * 8);
    version |= ((uint32_t)CIAM_VER_MINOR << 2 * 8);
    version |= ((uint32_t)CIAM_VER_PATCH << 1 * 8);
    version |= ((uint32_t)CIAM_VER_REV   << 0 * 8);
    header.version = version;

    encode_timestamp(&header.timestamp);
    header.code_size = code_size;
    header.const_size = 0x9988;
    header.main_addr = sizeof(ciam_header_t);
    memcpy(buff, &header, sizeof(ciam_header_t));

    size_t offset = sizeof(ciam_header_t);
    offset += encode_numeric_constants(buff + sizeof(ciam_header_t), &pool->numbers);
    size_t inst_code_length = encode_inst_code(buff + offset, code, code_size);
    ((ciam_header_t*)buff)->code_size = inst_code_length;
}

static void decode_numeric_constants(char* buff, const_num_table_t* num_table)
{
#define DECODE(X, size) do { memcpy(tmp, buff + offset, size); offset += size; } while (false) 
    char tmp[64];
    size_t offset = 0;
    DECODE(tmp, sizeof(uint16_t));
    num_table->count = *(uint16_t*)tmp;
    num_table->nums = (num_const_t*)malloc(sizeof(num_const_t) * num_table->count);
    for (uint16_t i = 0; i < num_table->count; i++)
    {
        DECODE(tmp, sizeof(uint8_t));
        num_table->nums[i].type = *(uint8_t*)tmp;
        DECODE(tmp, sizeof(uint64_t));
        num_table->nums[i].value = *(uint64_t*)tmp;
    }
#undef DECODE
}

void decode(char* buff, const_pool_t* pool)
{
    ciam_header_t header = {0};
    memcpy(&header, buff, sizeof(ciam_header_t));
    decode_numeric_constants(buff + sizeof(ciam_header_t), &pool->numbers);
}


#endif // COMMON_CODEC_H_
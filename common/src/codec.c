
#include "codec.h"


static int op_has_operand[] = {
#define X(kind, id, has_operand, label) [id] = has_operand,
    OPCODE_LIST
#undef X
};

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

static size_t encode_string_constants(char* buff, const_str_table_t* str_table)
{
#define ENCODE(X) do { memcpy(buff + offset, &X, sizeof(X)); offset += sizeof(X); } while (false)
    size_t offset = 0;
    ENCODE(str_table->count);
    for (size_t str_idx = 0; str_idx < str_table->count; str_idx++)
    {
        ENCODE(str_table->strings[str_idx].length);
        for (uint32_t i = 0; i < str_table->strings[str_idx].length; i++)
        {
            ENCODE(str_table->strings[str_idx].chars[i]);
        }
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

size_t encode(char* buff, module_t* module)
{
    ciam_header_t header;
    header.magic = CIAM_MAGIC_NUM;
    int32_t version = 0;
    version = ENCODE_CIAM_VERSION(version);
    header.version = version;

    encode_timestamp(&header.timestamp);
    header.code_size = module->code_size;
    header.const_size = 0x9988;
    header.main_addr = sizeof(ciam_header_t);
    memcpy(buff, &header, sizeof(ciam_header_t));

    size_t offset = sizeof(ciam_header_t);
    offset += encode_numeric_constants(buff + sizeof(ciam_header_t), &module->pool.numbers);
    offset += encode_string_constants(buff + offset, &module->pool.strings);
    size_t inst_code_length = encode_inst_code(buff + offset, module->code, module->code_size);
    ((ciam_header_t*)buff)->code_size = inst_code_length;
    return inst_code_length + offset;
}

static size_t decode_numeric_constants(char* buff, const_num_table_t* num_table)
{
#define DECODE(X, size) do { memcpy(X, buff + offset, size); offset += size; } while (false) 
    char tmp[64] = {0};
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
    return offset;
#undef DECODE
}

static size_t decode_string_constants(char* buff, const_str_table_t* str_table)
{
#define DECODE(X, size) do { memcpy(X, buff + offset, size); offset += size; } while (false) 
    uint16_t count;
    size_t offset = 0;
    DECODE(&count, sizeof(count));
    str_table->strings = (string_const_t*)calloc(count, sizeof(string_const_t));
    for (uint16_t i = 0; i < count; i++)
    {
        uint32_t length = 0;
        DECODE(&length, sizeof(length));
        str_table->strings[i].length = length;
        str_table->strings[i].chars = (char*)calloc(length, sizeof(char));
        DECODE(str_table->strings[i].chars, length);
    }
    return offset;
#undef DECODE
}

static void decode_timestamp(struct tm* timestamp, ts_t* raw_time)
{
    timestamp->tm_sec       = raw_time->sec;
    timestamp->tm_min       = raw_time->min;
    timestamp->tm_hour      = raw_time->hour;
    timestamp->tm_mday      = raw_time->m_day;
    timestamp->tm_mon       = raw_time->mon;
    timestamp->tm_year      = raw_time->year;
    timestamp->tm_wday      = raw_time->w_day;
    timestamp->tm_yday      = raw_time->y_day;
    timestamp->tm_isdst     = raw_time->is_dst;
}

static void decode_inst_code(char* buff, code_t** code, size_t code_size)
{
    size_t offset = 0;
    size_t index = 0;
    while (offset < code_size)
    {
        (*code)[index].op = (opcode_t)buff[offset];
        offset += 1;
        (*code)[index].opnd1 = (opnd_t)buff[offset];
        offset += 8;
        index++;
    }
}

void decode(char* buff, module_t* module)
{
    ciam_header_t header = {0};
    memcpy(&header, buff, sizeof(ciam_header_t));
    size_t offset = decode_numeric_constants(buff + sizeof(ciam_header_t), &module->pool.numbers);
    offset += decode_string_constants(buff + sizeof(ciam_header_t) + offset, &module->pool.strings);
    decode_timestamp(module->time_stamp, &header.timestamp);
    module->code_size = header.code_size;
    module->code = (code_t*)malloc(sizeof(code_t) * (module->code_size));
    decode_inst_code(buff + sizeof(ciam_header_t) + offset, &module->code, module->code_size);
}

#ifndef CIAM_READER_H_
#define CIAM_READER_H_

#include "code.h"
#include "common.h"

code_t* parse_code(const char* buffer, size_t length);
code_t* open_bc_source_file(const char* path);

#endif // CIAM_READER_H_
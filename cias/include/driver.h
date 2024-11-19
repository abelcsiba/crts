
#ifndef CIAS_DRIVER_H_
#define CIAS_DRIVER_H_

#include <stdbool.h>

typedef struct {
    char*           input;
    int             log_level;
    bool            compile_only;
    char*           full_path;
    char**          include_paths;
    char*           ciam_bc_name;
} driver_t;

void    init_driver(driver_t* driver);
char*   parse_args(driver_t* driver, int argc, char** argv);

#endif // CIAS_DRIVER_H_
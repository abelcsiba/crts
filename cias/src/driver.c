
#include "driver.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bits/getopt_core.h>

#define PATH_SIZE 256


static void print_usage(const char* prog_name)
{
    fprintf(stdout, "Usage: %s [options] sourcefile\n", prog_name);
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  -h               Show this help message and exit\n");
    fprintf(stdout, "  -v               Enable verbose mode\n");
    fprintf(stdout, "  -o <file>        Place the output file into <file>.ciam\n");
    fprintf(stdout, "  -C               Compile only, do not execute\n");
    fprintf(stdout, "  -I <directory>   Add directory to search for included libraries\n");
}

static int __attribute__((unused)) get_file_path_from_fd(int fd, char *path, size_t path_size) {
    char fd_path[200];
    snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d", fd);

    ssize_t len = readlink(fd_path, path, path_size - 1);
    if (len == -1) {
        perror("readlink");
        return -1;
    }

    path[len] = '\0';
    return 0;
}

static const __attribute__((unused)) char* get_file_name(const char *path) {
    const char *file_name = strrchr(path, '/');
    if (file_name) {
        return file_name + 1; // Move past the '/' character
    } else {
        return path; // No '/' found, the whole path is the file name
    }
}

void init_driver(driver_t* driver)
{
    driver->input = NULL;
    driver->log_level = 0;
    driver->compile_only = false;
    driver->full_path = NULL;
    driver->ciam_bc_name = NULL;
    driver->include_paths = NULL;
}

static void validate_args(driver_t* driver, const char* prog_name)
{
#define ABORT_WITH_ERROR(Msg) do { fprintf(stderr, "[Error] Missing input\n"); print_usage(prog_name); exit(EXIT_FAILURE); } while (false)
    
    if (NULL == driver->input) ABORT_WITH_ERROR("[Error] Missing input\n");

#undef ABORT_WITH_ERROR
}

static char* open_input(driver_t* driver, const char* prog_name)
{
#define ABORT_WITH_ERROR(Msg) do { fprintf(stderr, "[Error] Missing input\n"); print_usage(prog_name); exit(EXIT_FAILURE); } while (false)
    FILE* fp = fopen(driver->input, "rb");
    if (NULL == fp) ABORT_WITH_ERROR("Cannot open input file " input "\n");

    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* buff = (char*)calloc(length + 1, sizeof(char));
    fread(buff, sizeof(char), length, fp);

    driver->full_path = (char*)calloc(PATH_SIZE, sizeof(char));
    get_file_path_from_fd(fileno(fp), driver->full_path, PATH_SIZE);

    fclose(fp);

    return buff;
#undef ABORT_WITH_ERROR
}

char* parse_args(driver_t* driver, int argc, char** argv)
{
    int opt;

    if (argc < 2)
    {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    while ( (opt = getopt(argc, argv, "hvo:CI:")) != -1 )
    {
        switch (opt)
        {
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            case 'v':
                driver->log_level = 1;
                break;
            case 'o':
                driver->ciam_bc_name = optarg;
                break;
            case 'C':
                driver->compile_only = true;
                break;
            case 'I':
                driver->include_paths = (char**)malloc(sizeof(char*));
                driver->include_paths[0] = optarg;
                break;
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "Expected source file after options\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    driver->input = argv[optind];

    validate_args(driver, argv[0]);
    return open_input(driver, argv[0]);
}

#include "driver.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

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


#include "util.h"

#include <stdio.h>


void print_time(char* buff, struct tm* time)
{
    sprintf(buff, "%d-%d-%d %d:%d-%d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
}
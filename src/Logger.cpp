//
// Created by Davide Caroselli on 05/03/23.
//

#include <cstdarg>
#include <cstdio>
#include "Logger.h"

void Logger::err(const char *format, ...) {
    fprintf(stdout, "[ERROR] ");

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);

    fprintf(stdout, "\n");
}

void Logger::warn(const char *format, ...) {
    fprintf(stdout, "[WARN] ");

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);

    fprintf(stdout, "\n");
}

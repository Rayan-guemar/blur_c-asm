
#include "../include/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exit_error_message(const char* msg) {
    fflush(stdin);
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "error: %s\n", msg);
    exit(EXIT_FAILURE);
}

void exit_wrong_file_type(void) {
    exit_error_message("wrong file type (png required)");
}

void exit_error_while_reading_file(void) {
    exit_error_message("error while reading file");
}

void exit_memory_allocation_error(void) {
    exit_error_message("memory allocation error");
}

void exit_file_open_error(const char* filename) {
    char buf[256];
    snprintf(buf, sizeof(buf), "cannot open file: %s", filename);
    exit_error_message(buf);
}

void exit_file_write_error(const char* filename) {
    char buf[256];
    snprintf(buf, sizeof(buf), "cannot write to file: %s", filename);
    exit_error_message(buf);
}

void exit_file_read_error(const char* filename) {
    char buf[256];
    snprintf(buf, sizeof(buf), "cannot read from file: %s", filename);
    exit_error_message(buf);
}

void exit_invalid_chunk_error(const char* chunk_id) {
    char buf[64];
    snprintf(buf, sizeof(buf), "invalid chunk: %s", chunk_id);
    exit_error_message(buf);
}

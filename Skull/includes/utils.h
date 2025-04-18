#ifndef SKULL_UTILS_H
#define SKULL_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    int run_only;
    int compile_only;
    int keep_intermediate_files;
} skull_config_t;

char* read_file(const char* filename);
void write_file(const char* filename, char* buffer);

#ifdef SKULL_UTILS_H_IMPLEMENTATION

char* read_file(const char* filename) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file for reading '%s'\n", filename);
        return NULL;  // Return NULL instead of exiting
    }

    char* buffer = (char*) calloc(1, sizeof(char));
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        return NULL;
    }
    buffer[0] = '\0';

    while ((read = getline(&line, &len, fp)) != -1) {
        char* new_buffer = (char*) realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
        if (!new_buffer) {
            fprintf(stderr, "Memory reallocation failed\n");
            free(buffer);
            free(line);
            fclose(fp);
            return NULL;
        }
        buffer = new_buffer;
        strcat(buffer, line);
    }
    fclose(fp);
    if (line) free(line);

    return buffer;
}

void write_file(const char* filename, char* buffer) {
    FILE * fp;

    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file for writing '%s'\n", filename);
        return;
    }

    fputs(buffer, fp);
    fclose(fp);
}

#endif // SKULL_UTILS_H_IMPLEMENTATION
#endif // SKULL_UTILS_H
#ifndef SKULL_UTILS_H
#define SKULL_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

char* read_file(const char* filename);
void write_file(const char* filename, char* buffer);

#ifdef SKULL_UTILS_H_IMPLEMENTATION

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

char* read_file(const char* filename) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Could open file for reading '%s'\n", filename);
        exit(1);
    }

    char* buffer = (char*) calloc(1, sizeof(char));
    buffer[0] = '\0';

    while ((read = getline(&line, &len, fp)) != -1) {
        buffer = (char*) realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
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
        fprintf(stderr, "Could open file for writing '%s'\n", filename);
        exit(1);
    }

    fputs(buffer, fp);

    fclose(fp);
})

#endif // SKULL_UTILS_H_IMPLEMENTATION
#endif // SKULL_UTILS_H
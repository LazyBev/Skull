#ifndef SKULL_H
#define SKULL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h> // For access()

// Forward declarations to avoid circular dependencies
typedef struct lexerStruct lexer_t;
typedef struct parserStruct parser_t;
typedef struct astStruct ast_t;

// Include necessary headers without implementation defines
#include "utils.h"
#include "token.h"
#include "list.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "asm.h"

#define PATH_MAX_SIZE 4096

typedef struct {
    int keep_intermediate_files; // 0 = remove .asm file, 1 = keep it
} skull_config_t;

void skull_compile(char* src, const char* output_filename, skull_config_t config);
void skull_compile_file(const char* filename, const char* output_filename, skull_config_t config);
void extract_base_name_and_extension(const char* filename, char* base_name, size_t base_size, char* extension, size_t ext_size);
const char* skull_strerror(int err);

#ifdef SKULL_H_IMPLEMENTATION

// Safe string buffer for shell command output
typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} string_buffer_t;

static string_buffer_t init_string_buffer(size_t initial_capacity) {
    string_buffer_t buf = {0};
    buf.data = (char*)malloc(initial_capacity);
    if (!buf.data) {
        fprintf(stderr, "Memory allocation failed for string buffer\n");
        exit(1);
    }
    buf.data[0] = '\0';
    buf.size = 0;
    buf.capacity = initial_capacity;
    return buf;
}

static void append_string_buffer(string_buffer_t* buf, const char* str) {
    if (!buf || !str) return;
    
    size_t str_len = strlen(str);
    if (buf->size + str_len + 1 > buf->capacity) {
        size_t new_capacity = buf->capacity * 2 > buf->size + str_len + 1 ? buf->capacity * 2 : buf->size + str_len + 1;
        char* new_data = (char*)realloc(buf->data, new_capacity);
        if (!new_data) {
            fprintf(stderr, "Memory reallocation failed for string buffer\n");
            free(buf->data);
            exit(1);
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    strcpy(buf->data + buf->size, str);
    buf->size += str_len;
}

static void free_string_buffer(string_buffer_t* buf) {
    if (!buf) return;
    
    if (buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    buf->size = 0;
    buf->capacity = 0;
}

static char* sh(const char* binpath, const char* source) {
    if (!binpath || !source) {
        fprintf(stderr, "Invalid arguments to sh function\n");
        return NULL;
    }
    
    string_buffer_t output = init_string_buffer(1024);
    FILE* fp;
    char path[PATH_MAX_SIZE];
    
    // Use snprintf safely to construct command
    char* cmd = (char*)malloc(PATH_MAX_SIZE);
    if (!cmd) {
        fprintf(stderr, "Memory allocation failed for command buffer\n");
        free_string_buffer(&output);
        return NULL;
    }

    if (snprintf(cmd, PATH_MAX_SIZE, "%s %s", binpath, source) >= PATH_MAX_SIZE) {
        fprintf(stderr, "Command too long: %s %s\n", binpath, source);
        free(cmd);
        free_string_buffer(&output);
        return NULL;
    }

    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run command: %s (%s)\n", cmd, skull_strerror(errno));
        free(cmd);
        free_string_buffer(&output);
        return NULL;
    }

    while (fgets(path, sizeof(path), fp) != NULL) {
        append_string_buffer(&output, path);
    }

    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
        free(cmd);
        char* result = strdup(output.data);
        free_string_buffer(&output);
        return result;  // Return output even if command failed
    }

    free(cmd);
    char* result = strdup(output.data);
    free_string_buffer(&output);
    return result;
}

const char* skull_strerror(int err) {
    return strerror(err);
}

void extract_base_name_and_extension(const char* filename, char* base_name, size_t base_size, char* extension, size_t ext_size) {
    if (!filename || !base_name || !extension) return;
    
    // Initialize output buffers
    base_name[0] = '\0';
    extension[0] = '\0';
    
    const char* dot = strrchr(filename, '.');
    
    if (dot && dot != filename) {
        size_t base_len = dot - filename;
        if (base_len >= base_size) base_len = base_size - 1;
        strncpy(base_name, filename, base_len);
        base_name[base_len] = '\0';

        size_t ext_len = strlen(dot);
        if (ext_len >= ext_size) ext_len = ext_size - 1;
        strncpy(extension, dot, ext_len);
        extension[ext_len] = '\0';
    } else {
        strncpy(base_name, filename, base_size - 1);
        base_name[base_size - 1] = '\0';
    }
}

void skull_compile(char* src, const char* output_filename, skull_config_t config) {
    if (!src) {
        fprintf(stderr, "Error: Source code is NULL\n");
        return;
    }

    lexer_t* lexer = init_lexer(src);
    if (!lexer) {
        fprintf(stderr, "Error: Failed to initialize lexer\n");
        return;
    }

    parser_t* parser = init_parser(lexer);
    if (!parser) {
        fprintf(stderr, "Error: Failed to initialize parser\n");
        // Clean up lexer (note: we should have a free_lexer function)
        free(lexer);
        return;
    }

    ast_t* root = parse(parser);
    if (!root) {
        fprintf(stderr, "Error: Parsing failed, invalid syntax\n");
        // Clean up parser (note: we should have a free_parser function)
        free(parser);
        free(lexer);
        return;
    }

    const char* default_name = "main";
    char base_name[PATH_MAX_SIZE] = {0};
    char extension[PATH_MAX_SIZE] = {0};
    char executable_name[PATH_MAX_SIZE] = {0};
    char asm_filename[PATH_MAX_SIZE] = {0};
    
    if (output_filename) {
        extract_base_name_and_extension(output_filename, base_name, PATH_MAX_SIZE, extension, PATH_MAX_SIZE);

        if (extension[0] != '\0') {
            strncpy(executable_name, output_filename, PATH_MAX_SIZE - 1);
            executable_name[PATH_MAX_SIZE - 1] = '\0';
        } else {
            strncpy(base_name, output_filename, PATH_MAX_SIZE - 1);
            base_name[PATH_MAX_SIZE - 1] = '\0';
        }
    } else {
        strncpy(base_name, default_name, PATH_MAX_SIZE - 1);
        base_name[PATH_MAX_SIZE - 1] = '\0';
    }

    if (executable_name[0] == '\0') {
        strncpy(executable_name, base_name, PATH_MAX_SIZE - 1);
        executable_name[PATH_MAX_SIZE - 1] = '\0';
    }
    
    if (snprintf(asm_filename, PATH_MAX_SIZE, "%s.asm", base_name) >= PATH_MAX_SIZE) {
        fprintf(stderr, "Error: Assembly filename too long\n");
        // Clean up resources
        // Add proper free functions for these types
        free(root);
        free(parser);
        free(lexer);
        return;
    }
    
    char* s = asm_f_root(root);
    if (!s) {
        fprintf(stderr, "Error: Failed to generate assembly code\n");
        free(root);
        free(parser);
        free(lexer);
        return;
    }

    write_file(asm_filename, s);
    // Verify the file was written successfully
    if (access(asm_filename, F_OK) != 0) {
        fprintf(stderr, "Error: Failed to write assembly file %s (%s)\n", asm_filename, skull_strerror(errno));
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }

    char gcc_cmd[PATH_MAX_SIZE * 2];
    if (snprintf(gcc_cmd, sizeof(gcc_cmd), "-o %s %s", executable_name, asm_filename) >= sizeof(gcc_cmd)) {
        fprintf(stderr, "Error: GCC command too long\n");
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }
    
    char* gcc_output = sh("gcc", gcc_cmd);
    if (!gcc_output) {
        fprintf(stderr, "Error: Failed to execute gcc command\n");
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }
    
    if (strlen(gcc_output) > 0) {
        fprintf(stderr, "Compilation error: %s\n", gcc_output);
        free(gcc_output);
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }
    free(gcc_output);

    if (!config.keep_intermediate_files) {
        if (remove(asm_filename) != 0) {
            fprintf(stderr, "Warning: Failed to remove %s (%s)\n", asm_filename, skull_strerror(errno));
        }
    }

    free(s);
    free(root);
    free(parser);
    free(lexer);
}

void skull_compile_file(const char* filename, const char* output_filename, skull_config_t config) {
    if (!filename) {
        fprintf(stderr, "Error: Input filename is NULL\n");
        return;
    }
    
    char* src = read_file(filename);
    if (!src) {
        fprintf(stderr, "Error: Failed to read file %s (%s)\n", filename, skull_strerror(errno));
        return;
    }
    
    skull_compile(src, output_filename, config);
    free(src);
}

#endif // SKULL_H_IMPLEMENTATION
#endif // SKULL_H
#ifndef SKULL_H
#define SKULL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef struct lexerStruct lexer_t;
typedef struct parserStruct parser_t;
typedef struct astStruct ast_t;

#include "utils.h"
#include "token.h"
#include "list.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "asm.h"

#define PATH_MAX_SIZE 4096

void skull_compile(char* src, const char* output_filename, bool keep_files);
void skull_compile_file(const char* filename, const char* output_filename, bool keep_files);
void extract_base_name_and_extension(const char* filename, char* base_name, size_t base_size, char* extension, size_t ext_size);
const char* skull_strerror(int err);

#ifdef SKULL_H_IMPLEMENTATION

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
        fprintf(stderr, "Command failed with status %d: %s\n", WEXITSTATUS(status), cmd);
        free(cmd);
        free_string_buffer(&output);
        return NULL; // Return NULL on failure
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

void skull_compile(char* src, const char* output_filename, bool keep_files) {
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
        free(lexer);
        return;
    }

    ast_t* root = parse(parser);
    if (!root) {
        fprintf(stderr, "Error: Parsing failed, invalid syntax\n");
        free(parser);
        free(lexer);
        return;
    }

    const char* default_name = "main";
    char base_name[PATH_MAX_SIZE] = {0};
    char extension[PATH_MAX_SIZE] = {0};
    char executable_name[PATH_MAX_SIZE] = {0};
    char asm_filename[PATH_MAX_SIZE] = {0};
    char obj_filename[PATH_MAX_SIZE] = {0};

    if (output_filename) {
        extract_base_name_and_extension(output_filename, base_name, PATH_MAX_SIZE, extension, PATH_MAX_SIZE);
        strncpy(executable_name, output_filename, PATH_MAX_SIZE - 1);
        executable_name[PATH_MAX_SIZE - 1] = '\0';
    } else {
        strncpy(base_name, default_name, PATH_MAX_SIZE - 1);
        base_name[PATH_MAX_SIZE - 1] = '\0';
        strncpy(executable_name, default_name, PATH_MAX_SIZE - 1);
        executable_name[PATH_MAX_SIZE - 1] = '\0';
    }

    // Use unique names for intermediate files
    if (snprintf(asm_filename, PATH_MAX_SIZE, "%s.asm", base_name) >= PATH_MAX_SIZE) {
        fprintf(stderr, "Error: Assembly filename too long\n");
        free(root);
        free(parser);
        free(lexer);
        return;
    }
    if (snprintf(obj_filename, PATH_MAX_SIZE, "%s.o", base_name) >= PATH_MAX_SIZE) {
        fprintf(stderr, "Error: Object filename too long\n");
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
    if (access(asm_filename, F_OK) != 0) {
        fprintf(stderr, "Error: Failed to write assembly file %s (%s)\n", asm_filename, skull_strerror(errno));
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }

    // Assemble the .asm file to an object file using nasm
    char nasm_cmd[PATH_MAX_SIZE * 2];
    if (snprintf(nasm_cmd, sizeof(nasm_cmd), "-felf64 %s -o %s", asm_filename, obj_filename) >= sizeof(nasm_cmd)) {
        fprintf(stderr, "Error: NASM command too long\n");
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }

    char* nasm_output = sh("nasm", nasm_cmd);
    if (!nasm_output || strlen(nasm_output) > 0) {
        fprintf(stderr, "Error: Failed to assemble %s: %s\n", asm_filename, nasm_output ? nasm_output : "unknown error");
        free(nasm_output);
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }
    free(nasm_output);

    // Link the object file to create the executable using ld
    char ld_cmd[PATH_MAX_SIZE * 2];
    if (snprintf(ld_cmd, sizeof(ld_cmd), "-e _start %s -o %s", obj_filename, executable_name) >= sizeof(ld_cmd)) {
        fprintf(stderr, "Error: LD command too long\n");
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }

    char* ld_output = sh("ld", ld_cmd);
    if (!ld_output || strlen(ld_output) > 0) {
        fprintf(stderr, "Error: Failed to link %s: %s\n", obj_filename, ld_output ? ld_output : "unknown error");
        free(ld_output);
        free(s);
        free(root);
        free(parser);
        free(lexer);
        return;
    }
    free(ld_output);

    if (!keep_files) {
        if (remove(asm_filename) != 0) {
            fprintf(stderr, "Warning: Failed to remove %s (%s)\n", asm_filename, skull_strerror(errno));
        }
        
        if (remove(obj_filename) != 0) {
            fprintf(stderr, "Warning: Failed to remove %s (%s)\n", obj_filename, skull_strerror(errno));
        }
    }

    free(s);
    free(root);
    free(parser);
    free(lexer);
}

// Fix 7: Enhanced skull_compile_file with better error handling
void skull_compile_file(const char* filename, const char* output_filename, bool keep_files) {
    if (!filename) {
        fprintf(stderr, "Error: Input filename is NULL\n");
        return;
    }
    
    char* src = read_file(filename);
    if (!src) {
        fprintf(stderr, "Error: Failed to read file %s (%s)\n", filename, skull_strerror(errno));
        return;
    }
    
    // Add additional information for debugging
    printf("Compiling file: %s\n", filename);
    if (output_filename) {
        printf("Output executable: %s\n", output_filename);
    }
    
    skull_compile(src, output_filename, keep_files);
    free(src);
}

#endif // SKULL_H_IMPLEMENTATION
#endif // SKULL_H
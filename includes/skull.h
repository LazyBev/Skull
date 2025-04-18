#ifndef SKULL_H
#define SKULL_H

#define SKULL_LEXER_H_IMPLEMENTATION
#define SKULL_UTILS_H_IMPLEMENTATION
#define SKULL_PARSER_H_IMPLEMENTATION
#define SKULL_ASM_H_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "asm.h"

#define PATH_MAX_SIZE 1024

void skull_compile(char* src, const char* output_filename);
void skull_compile_file(const char* filename, const char* output_filename);
void extract_base_name_and_extension(const char* filename, char* base_name, size_t base_size, char* extension, size_t ext_size);

#ifdef SKULL_H_IMPLEMENTATION

static char* sh(char* binpath, char* source) {
    char* output = (char*)calloc(1, sizeof(char));
    output[0] = '\0';

    FILE *fp;
    char path[PATH_MAX_SIZE];
    char* cmd = (char*)calloc(PATH_MAX_SIZE, sizeof(char));

    snprintf(cmd, PATH_MAX_SIZE, "%s %s", binpath, source);

    fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Failed to run command: %s\n", cmd);
        free(cmd);
        free(output);
        exit(1);
    }

    while (fgets(path, sizeof(path), fp) != NULL) {
        output = (char*)realloc(output, strlen(output) + strlen(path) + 1 * sizeof(char));
        strcat(output, path);
    }

    pclose(fp);
    free(cmd);
    return output;
}

void extract_base_name_and_extension(const char* filename, char* base_name, size_t base_size, char* extension, size_t ext_size) {
    const char* dot = strrchr(filename, '.');
    
    if (dot && dot != filename) {
        size_t base_len = dot - filename;
        if (base_len >= base_size) base_len = base_size - 1;
        strncpy(base_name, filename, base_len);
        base_name[base_len] = '\0';

        strncpy(extension, dot, ext_size - 1);
        extension[ext_size - 1] = '\0';
    } else {
        strncpy(base_name, filename, base_size - 1);
        base_name[base_size - 1] = '\0';
        extension[0] = '\0';
    }
}

void skull_compile(char* src, const char* output_filename) {
    lexer_t* lexer = init_lexer(src);
    parser_t* parser = init_parser(lexer);
    ast_t* root = parse(parser);

    const char* default_name = "main";
    char base_name[PATH_MAX_SIZE] = {0};
    char extension[PATH_MAX_SIZE] = {0};
    char executable_name[PATH_MAX_SIZE] = {0};
    char asm_filename[PATH_MAX_SIZE] = {0};
    char obj_filename[PATH_MAX_SIZE] = {0};
    
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
    
    snprintf(asm_filename, PATH_MAX_SIZE, "%s.asm", base_name);
    snprintf(obj_filename, PATH_MAX_SIZE, "%s.o", base_name);
    

    char* s = asm_f_root(root);
    write_file(asm_filename, s);

    char nasm_cmd[PATH_MAX_SIZE];
    snprintf(nasm_cmd, PATH_MAX_SIZE, "-f elf64 %s -o %s", asm_filename, obj_filename);
    
    char* nasm_output = sh("nasm", nasm_cmd);
    if (strlen(nasm_output) > 0) {
        printf("Assembly error: %s\n", nasm_output);
        free(nasm_output);
        free(s);
        exit(1);
    }
    free(nasm_output);

    char ld_cmd[PATH_MAX_SIZE];
    snprintf(ld_cmd, PATH_MAX_SIZE, "%s -o %s", obj_filename, executable_name);
    
    char* ld_output = sh("ld", ld_cmd);
    if (strlen(ld_output) > 0) {
        printf("Linking error: %s\n", ld_output);
        free(ld_output);
        free(s);
        exit(1);
    }
    free(ld_output);

    remove(obj_filename);
    remove(asm_filename);

    free(s);
}

void skull_compile_file(const char* filename, const char* output_filename) {
    char* src = read_file(filename);
    skull_compile(src, output_filename);
    free(src);
}

#endif // SKULL_H_IMPLEMENTATION
#endif // SKULL_H

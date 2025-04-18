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

void skull_compile(char* src, const char* output_filename);
void skull_compile_file(const char* filename, const char* output_filename);

#ifdef SKULL_H_IMPLEMENTATION

static char* sh(char* binpath, char* source) {
    char* output = (char*)calloc(1, sizeof(char));
    output[0] = '\0';

    FILE *fp;
    char path[1035];
    char* cmd_template = "%s %s"; // binpath is the command, source is arguments
    char* cmd = (char*)calloc(strlen(cmd_template) + strlen(binpath) + strlen(source) + 128, sizeof(char));
    sprintf(cmd, cmd_template, binpath, source);

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

void skull_compile(char* src, const char* output_filename) {
    lexer_t* lexer = init_lexer(src);
    parser_t* parser = init_parser(lexer);
    ast_t* root = parse(parser);

    char* s = asm_f_root(root);
    write_file("a.asm", s);

    // Default output filename if none provided
    const char* out_file = output_filename ? output_filename : "a.out";

    // Step 1: Assemble a.asm to a.o using nasm
    char* nasm_output = sh("nasm", "-f elf64 a.asm -o a.o");
    if (strlen(nasm_output) > 0) {
        printf("Assembly error: %s\n", nasm_output);
        free(nasm_output);
        free(s);
        exit(1);
    }
    free(nasm_output);

    // Step 2: Link a.o to executable using ld
    char ld_cmd[256];
    snprintf(ld_cmd, sizeof(ld_cmd), "a.o -o %s", out_file);
    char* ld_output = sh("ld", ld_cmd);
    if (strlen(ld_output) > 0) {
        printf("Linking error: %s\n", ld_output);
        free(ld_output);
        free(s);
        exit(1);
    }
    free(ld_output);

    // Clean up temporary files
    remove("a.o");
    remove("a.asm");

    // Free allocated memory
    free(s);
}

void skull_compile_file(const char* filename, const char* output_filename) {
    char* src = read_file(filename);
    skull_compile(src, output_filename);
    free(src);
}

#endif // SKULL_H_IMPLEMENTATION
#endif // SKULL_H
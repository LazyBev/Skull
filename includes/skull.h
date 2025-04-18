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
void skull_compile_file(const char* filename);

#ifdef SKULL_H_IMPLEMENTATION

void skull_compile(char* src, const char* output_filename) {
    lexer_t* lexer = init_lexer(src);
    parser_t* parser = init_parser(lexer);
    ast_t* root = parse(parser);

    char* assembly_code = asm_f(root);
    
    FILE* output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error: Could not create output file '%s'\n", output_filename);
        exit(1);
    }
    
    fprintf(output_file, "%s", assembly_code);
    fclose(output_file);
    
    free(assembly_code);
}

void skull_compile_file(const char* filename) {
    char* src = read_file(filename);
    
    char* output_filename = strdup(filename);
    char* extension = strrchr(output_filename, '.');
    
    if (extension != NULL) {
        *extension = '\0';
    }
    
    size_t new_len = strlen(output_filename) + 6;
    char* asm_filename = malloc(new_len);
    
    if (asm_filename == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(output_filename);
        free(src);
        exit(1);
    }
    
    sprintf(asm_filename, "%s.asm", output_filename);
    
    skull_compile(src, asm_filename);
    
    printf("Compiled %s to %s\n", filename, asm_filename);
    
    free(asm_filename);
    free(output_filename);
    free(src);
}

#endif // SKULL_H_IMPLEMENTATION
#endif // SKULL_H
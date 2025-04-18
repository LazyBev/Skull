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

void skull_compile(char* src);
void skull_compile_file(const char* filename);

#ifdef SKULL_H_IMPLEMENTATION

void skull_compile(char* src) {
    lexer_t* lexer = init_lexer(src);
    parser_t* parser = init_parser(lexer);
    ast_t* root = parse(parser);

    char* s = asm_f(root);
    printf("%s\n", s);
}

void skull_compile_file(const char* filename) {
    char* src = read_file(filename);
    skull_compile(src);
    free(src);
}

#endif // SKULL_H_IMPLEMENTATION
#endif // SKULL_H
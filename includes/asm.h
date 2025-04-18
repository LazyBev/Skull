#ifndef SKULL_ASM_H
#define SKULL_ASM_H

#define SKULL_AST_H_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

char* asm_f(ast_t* ast);
char* asm_f_compound(ast_t* ast);
char* asm_f_assignment(ast_t* ast);
char* asm_f_variable(ast_t* ast);
char* asm_f_call(ast_t* ast);
char* asm_f_int(ast_t* ast);

#ifdef SKULL_ASM_H_IMPLEMENTATION

char* asm_f_compound(ast_t* ast) {
    char* value = calloc(1, sizeof(char));
    for (int i = 0; i < (int) ast->children->size; i++) {
        ast_t* child_ast = (ast_t*) ast->children->items[i];
        char* next_value = asm_f(child_ast);
        value = realloc(value, (strlen(next_value) + 1) * sizeof(char));
        strcpy(value, next_value);
    }

    return value;
}

char* asm_f_assignment(ast_t* ast) {
    char* s = calloc(1, sizeof(char));
    if (ast->value->type == AST_FUNCTION) {
        const char* template = ".global %s\n%s:\n";
        s = realloc(s, (strlen(template) + (strlen(ast->name) * 2) + 1) * sizeof(char));
        sprintf(s, template, ast->name, ast->name);

        ast_t* as_val = ast->value;
        
        char* as_val_val = asm_f(as_val->value);
        s = realloc(s, (strlen(s) + strlen(as_val_val) + 1) * sizeof(char));
        strcat(s, as_val_val);
    }
    return s;
}

char* asm_f_variable(ast_t* ast) {}

char* asm_f_call(ast_t* ast) {
    if (strcmp(ast->name, "return") == 0) {

    }
}

char* asm_f_int(ast_t* ast) {}

char* asm_f(ast_t* ast) {
    char* value = calloc(1, sizeof(char));
    char* next_value = 0;
    switch (ast->type) {
        case AST_COMPOUND: next_value = asm_f_compound(ast); break;
        case AST_ASSIGNMENT: next_value = asm_f_assignment(ast); break;
        case AST_VARIABLE: next_value = asm_f_variable(ast); break;
        case AST_CALL: next_value = asm_f_call(ast); break;
        case AST_INT: next_value = asm_f_int(ast); break;
        default: {printf("ERROR: No frontend for AST of type: '%d'\n", ast->type); exit(1);} break;
    }
    value = realloc(value, (strlen(next_value) + 1) * sizeof(char));
    strcat(value, next_value);

    return value;
}

#endif // SKULL_ASM_H_IMPLEMENTATION
#endif // SKULL_ASM_H
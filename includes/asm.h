#ifndef SKULL_ASM_H
#define SKULL_ASM_H

#define SKULL_AST_H_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

char* asm_f_compound(ast_t* ast);
char* asm_f_assignment(ast_t* ast);
char* asm_f_variable(ast_t* ast);
char* asm_f_call(ast_t* ast);
char* asm_f_int(ast_t* ast);
char* asm_f_root(ast_t* ast);
char* asm_f(ast_t* ast);

#ifdef SKULL_ASM_H_IMPLEMENTATION

char* asm_f_compound(ast_t* ast) {
    char* value = calloc(1, sizeof(char));
    for (int i = 0; i < (int) ast->children->size; i++) {
        ast_t* child_ast = (ast_t*) ast->children->items[i];
        char* next_value = asm_f(child_ast);
        value = realloc(value, (strlen(value) + strlen(next_value) + 1) * sizeof(char));
        strcat(value, next_value);
        free(next_value);  // Free the memory allocated by asm_f
    }

    return value;
}

char* asm_f_assignment(ast_t* ast) {
    char* s = calloc(1, sizeof(char));
    if (ast->value && ast->value->type == AST_FUNCTION) {
        const char* template = ".global %s\n"
                               "%s:\n";
        s = realloc(s, (strlen(template) + (strlen(ast->name) * 2) + 1) * sizeof(char));
        sprintf(s, template, ast->name, ast->name);

        ast_t* as_val = ast->value;
        
        char* as_val_val = asm_f(as_val->value);
        s = realloc(s, (strlen(s) + strlen(as_val_val) + 1) * sizeof(char));
        strcat(s, as_val_val);
        free(as_val_val);  // Free the memory allocated by asm_f
    }
    return s;
}

char* asm_f_variable(ast_t* ast) {
    return strdup("");
}

char* asm_f_call(ast_t* ast) {
    char* s = calloc(1, sizeof(char));
    if (strcmp(ast->name, "return") == 0) {
        int return_value = 0;
        
        if (ast->value) {
            if (ast->value->type == AST_INT) {
                return_value = ast->value->int_value;
            }
        }
        const char* template = "    mov $%d, %%eax\n"    
                               "    ret\n";
        char* ret_s = calloc(strlen(template) + 128, sizeof(char));
        sprintf(ret_s, template, return_value);
        s = realloc(s, (strlen(ret_s) + 1) * sizeof(char));
        strcat(s, ret_s);
        free(ret_s);
    }

    return s;
}

char* asm_f_int(ast_t* ast) {
    return strdup("");
}

char* asm_f_root(ast_t* ast) {
    const char* section_text = ".section .text\n"
                               ".global _start\n"
                               "_start:\n"
                               "call main\n"
                               "mov \%eax, \%ebx\n"
                               "mov $1, \%eax\n"
                               "int $0x80\n\n";
    char* value = (char *) calloc((strlen(section_text) + 128), sizeof(char));
    strcpy(value, section_text);

    char* next_value = asm_f(ast);
    value = (char *) realloc(value, (strlen(value) + strlen(next_value) + 1) * sizeof(char));
    strcat(value, next_value);
    free(next_value);

    return value;
}

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
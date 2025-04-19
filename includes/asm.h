#ifndef SKULL_ASM_H
#define SKULL_ASM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

char* asm_f_compound(ast_t* ast);
char* asm_f_assignment(ast_t* ast);
char* asm_f_variable(ast_t* ast, int id);
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
        value = realloc(value, strlen(value) + strlen(next_value) + 1);
        strcat(value, next_value);
        free(next_value);
    }

    return value;
}

char* asm_f_assignment(ast_t* ast) {
    char* s = calloc(1, sizeof(char));
    if (ast->value && ast->value->type == AST_FUNCTION) {
        const char* template = "global %s\n"
                               "%s:\n"
                               "    push rbp\n"
                               "    mov rsp, rbp\n";
        s = realloc(s, (strlen(template) + (strlen(ast->name)*2) + 1) * sizeof(char));
        sprintf(s, template, ast->name, ast->name);

        ast_t* as_val = ast->value;
        char* as_val_val = asm_f(as_val->value);

        s = realloc(s, strlen(s) + strlen(as_val_val) + 1);
        strcat(s, as_val_val);
        free(as_val_val);
    }
    return s;
}

char* asm_f_variable(ast_t* ast, int id) {
    char* s = calloc(1, sizeof(char));
    if (ast->type == AST_INT) {
        const char* template = "%d";
        s = realloc(s, (strlen(template) + 256) * sizeof(char));
        sprintf(s, template, ast->int_value);
    } else {
        const char* template = "%d(rsp)";
        s = realloc(s, (strlen(template) + 8) * sizeof(char));
        sprintf(s, template, id);
    }
    return s;
}

char* asm_f_call(ast_t* ast) {
    char* s = calloc(1, sizeof(char));
    if (strcmp(ast->name, "return") == 0) {
        ast_t* first_arg = (ast_t*) ast->value->children->size ? ast->value->children->items[0] : (void*) 0;
        char* var_s = calloc(2, sizeof(char));
        var_s[0] = '0';
        var_s[1] = '\0';

        if (first_arg && first_arg->type == AST_VARIABLE) {
            char* as_var_s = asm_f_variable(first_arg, 0);
            var_s = realloc(var_s, (strlen(as_var_s) + 1) * sizeof(char));
            strcpy(var_s, as_var_s);
            free(as_var_s);
        }
        const char* template = "    mov %s, rax\n"
                               "    mov rbp, rsp\n"
                               "    pop rbp\n\n"
                               "    ret\n";
        char* ret_s = calloc(strlen(template) + 128, sizeof(char));
        sprintf(ret_s, template, first_arg ? first_arg->int_value : 0);
        s = realloc(s, (strlen(ret_s) + 1) * sizeof(char));
        strcat(s, ret_s);
        free(ret_s);
    }

    return s;
}

char* asm_f_int(ast_t* ast) {
    // Not directly emitted unless in context
    return strdup("");
}

char* asm_f_root(ast_t* ast) {
    const char* section_text = "section .text\n"
                               "global _start\n"
                               "_start:\n"
                               "    mov rdi, [rsp]        ; argc\n"
                               "    lea rsi, [rsp+8]      ; argv\n"
                               "    call main\n"
                               "    mov rdi, rax\n"
                               "    mov rax, 60\n"
                               "    syscall\n\n";

    char* value = calloc(strlen(section_text) + 1, sizeof(char));
    strcpy(value, section_text);

    char* next_value = asm_f(ast);
    value = realloc(value, strlen(value) + strlen(next_value) + 1);
    strcat(value, next_value);
    free(next_value);

    return value;
}

char* asm_f(ast_t* ast) {
    char* next_value = NULL;

    switch (ast->type) {
        case AST_COMPOUND:   next_value = asm_f_compound(ast); break;
        case AST_ASSIGNMENT: next_value = asm_f_assignment(ast); break;
        case AST_VARIABLE:   next_value = asm_f_variable(ast, 0); break;
        case AST_CALL:       next_value = asm_f_call(ast); break;
        case AST_INT:        next_value = asm_f_int(ast); break;
        default:
            fprintf(stderr, "ERROR: No frontend for AST type: '%d'\n", ast->type);
            exit(1);
    }

    return next_value;
}

#endif // SKULL_ASM_H_IMPLEMENTATION
#endif // SKULL_ASM_H

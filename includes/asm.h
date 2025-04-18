#ifndef SKULL_ASM_H
#define SKULL_ASM_H

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
        if (!value) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
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
                               "    push %%rbp\n"
                               "    mov %%rsp, %%rbp\n";
        s = realloc(s, (strlen(template) + (strlen(ast->name) * 2) + 1) * sizeof(char));
        if (!s) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
        sprintf(s, template, ast->name, ast->name);
        
        ast_t* as_val = ast->value;
        
        char* as_val_val = asm_f(as_val->value);

        s = realloc(s, (strlen(s) + strlen(as_val_val) + 1) * sizeof(char));
        if (!s) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
        strcat(s, as_val_val);

        // Adding missing template_end
        const char* template_end = "    mov %%rbp, %%rsp\n"
                                   "    pop %%rbp\n"
                                   "    ret\n";
        s = realloc(s, (strlen(s) + strlen(template_end) + 1) * sizeof(char));
        if (!s) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
        strcat(s, template_end);
    }
    return s;
}

char* asm_f_variable(ast_t* ast) {
    // Complete implementation for variable handling
    char* s = calloc(128, sizeof(char));
    if (!s) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    // Subtract 8 bytes from stack pointer for a 64-bit variable
    snprintf(s, 128, "    sub %%rsp, 8  ; Allocate space for variable %s\n", ast->name ? ast->name : "");
    return s;
}

char* asm_f_call(ast_t* ast) {
    char* s = calloc(128, sizeof(char));
    if (strcmp(ast->name, "return") == 0) {
        ast_t* first_arg = (ast_t*) ast->value->children->size ? ast->value->children->items[0] : (void*) 0;
        const char* template = "    mov %%rbp, %%rsp\n"
                               "    pop %%rbp\n\n"
                               "    mov %d, %%rax\n"
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
    char* s = calloc(128, sizeof(char));
    if (!s) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    snprintf(s, 128, "    mov %%rax, %d    ; Load integer %d\n", ast->int_value, ast->int_value);
    return s;
}

char* asm_f_root(ast_t* ast) {
    const char* section_text = "section .text\n"
                               "global _start\n"
                               "_start:\n"
                               "    mov (%%rsp), %%rdi\n"
                               "    lea 8(%%rsp), %%rsi\n"
                               "    and $~15, %%rsp\n"
                               "    call main\n"
                               "    mov %%rax, %%rdi\n"
                               "    mov $60, %%rax\n"
                               "    syscall\n";
    char* value = (char *) calloc((strlen(section_text) + 128), sizeof(char));
    if (!value) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strcpy(value, section_text);

    char* next_value = asm_f(ast);
    value = (char *) realloc(value, (strlen(value) + strlen(next_value) + 1) * sizeof(char));
    if (!value) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(1);
    }
    strcat(value, next_value);
    free(next_value);

    return value;
}

char* asm_f(ast_t* ast) {
    char* value = calloc(1, sizeof(char));
    if (!value) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    char* next_value = NULL;

    switch (ast->type) {
        case AST_COMPOUND: next_value = asm_f_compound(ast); break;
        case AST_ASSIGNMENT: next_value = asm_f_assignment(ast); break;
        case AST_VARIABLE: next_value = asm_f_variable(ast); break;
        case AST_CALL: next_value = asm_f_call(ast); break;
        case AST_INT: next_value = asm_f_int(ast); break;
        default: {
            fprintf(stderr, "ERROR: No frontend for AST of type: '%d'\n", ast->type);
            exit(1);
        } break;
    }
    value = realloc(value, (strlen(next_value) + 1) * sizeof(char));
    if (!value) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(1);
    }
    strcat(value, next_value);
    free(next_value);

    return value;
}

#endif // SKULL_ASM_H_IMPLEMENTATION
#endif // SKULL_ASM_H
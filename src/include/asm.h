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
char* asm_f_function(ast_t* ast);
char* asm_f_statement(ast_t* ast);

#ifdef SKULL_ASM_H_IMPLEMENTATION

// Helper function to generate header for FASM output
char* generate_fasm_header() {
    return strdup(
        "format ELF64 executable 3\n"
        "entry main\n\n"
        "segment readable executable\n\n"
    );
}

char* asm_f_compound(ast_t* ast) {
    if (!ast || !ast->children) {
        return strdup("");
    }
    
    char* value = calloc(1, sizeof(char));
    value[0] = '\0';
    
    for (int i = 0; i < (int) ast->children->size; i++) {
        ast_t* child_ast = (ast_t*) ast->children->items[i];
        if (!child_ast) continue;
        
        char* next_value = asm_f(child_ast);
        if (next_value) {
            size_t new_len = strlen(value) + strlen(next_value) + 2; // +2 for newline and null terminator
            value = realloc(value, new_len * sizeof(char));
            strcat(value, next_value);
            strcat(value, "\n"); // Add newline between instructions
            free(next_value);
        }
    }

    return value;
}

char* asm_f_assignment(ast_t* ast) {
    if (!ast || !ast->name) {
        return strdup("; Invalid assignment");
    }
    
    char* template = "; Assignment: %s";
    char* s = calloc(strlen(template) + strlen(ast->name) + 1, sizeof(char));
    sprintf(s, template, ast->name);
    
    char* value_asm = ast->value ? asm_f(ast->value) : strdup("null");
    char* result = calloc(strlen(s) + strlen(value_asm) + 32, sizeof(char));
    sprintf(result, "%s\nmov eax, [%s]", value_asm, ast->name);
    
    free(s);
    free(value_asm);
    
    return result;
}

char* asm_f_variable(ast_t* ast) {
    if (!ast || !ast->name) {
        return strdup("; Invalid variable");
    }
    
    char* s = calloc(32 + strlen(ast->name), sizeof(char));
    sprintf(s, "; Variable: %s", ast->name);
    return s;
}

char* asm_f_call(ast_t* ast) {
    if (!ast || !ast->name) {
        return strdup("; Invalid function call");
    }
    
    char* args_asm = "";
    if (ast->value && ast->value->type == AST_COMPOUND) {
        args_asm = asm_f(ast->value);
    }
    
    // Simple function call assembly in FASM format
    char* template = "; Function call: %s\n%s\ncall %s";
    char* s = calloc(strlen(template) + strlen(ast->name) + strlen(args_asm) + 1, sizeof(char));
    sprintf(s, template, ast->name, args_asm, ast->name);
    
    if (strcmp(args_asm, "") != 0) {
        free(args_asm);
    }
    
    return s;
}

char* asm_f_int(ast_t* ast) {
    char* s = calloc(32, sizeof(char));
    sprintf(s, "mov eax, %d", ast->int_value);
    return s;
}

char* asm_f_statement(ast_t* ast) {
    if (!ast || !ast->name) {
        return strdup("; Invalid statement");
    }
    
    if (strcmp(ast->name, "return") == 0) {
        char* value_asm = ast->value ? asm_f(ast->value) : strdup("mov eax, 0");
        char* result = calloc(strlen(value_asm) + 32, sizeof(char));
        sprintf(result, "%s\nret", value_asm);
        free(value_asm);
        return result;
    }
    
    return strdup("; Unknown statement");
}

char* asm_f_function(ast_t* ast) {
    if (!ast || !ast->name) {
        return strdup("; Invalid function definition");
    }
    
    // Function declaration in FASM format
    char* func_start = calloc(128 + strlen(ast->name), sizeof(char));
    sprintf(func_start, "%s:\n\tpush rbp\n\tmov rbp, rsp", ast->name);
    
    // Function body
    char* body_asm = "";
    if (ast->value && ast->value->type == AST_COMPOUND) {
        body_asm = asm_f(ast->value);
        // Add indentation to body
        char* indented_body = calloc(strlen(body_asm) * 2, sizeof(char));
        char* body_copy = strdup(body_asm);
        char* line = strtok(body_copy, "\n");
        while (line) {
            strcat(indented_body, "\t");
            strcat(indented_body, line);
            strcat(indented_body, "\n");
            line = strtok(NULL, "\n");
        }
        free(body_copy);
        free(body_asm);
        body_asm = indented_body;
    }
    
    // Function epilogue (if no explicit return)
    char* func_end = "\tmov rsp, rbp\n\tpop rbp\n\tret";
    
    // Combine all parts
    char* result = calloc(strlen(func_start) + strlen(body_asm) + strlen(func_end) + 10, sizeof(char));
    sprintf(result, "%s\n%s%s", func_start, body_asm, func_end);
    
    if (strcmp(body_asm, "") != 0) {
        free(body_asm);
    }
    
    return result;
}

char* asm_f(ast_t* ast) {
    static int is_first_call = 1;
    char* result;
    
    if (is_first_call) {
        is_first_call = 0;
        char* header = generate_fasm_header();
        char* content = ast ? asm_f(ast) : strdup("; Empty AST");
        
        result = calloc(strlen(header) + strlen(content) + 10, sizeof(char));
        sprintf(result, "%s%s", header, content);
        
        free(header);
        free(content);
        return result;
    }
    
    if (!ast) {
        return strdup("; Null AST node");
    }
    
    switch (ast->type) {
        case AST_COMPOUND: return asm_f_compound(ast);
        case AST_ASSIGNMENT: return asm_f_assignment(ast);
        case AST_VARIABLE: return asm_f_variable(ast);
        case AST_CALL: return asm_f_call(ast);
        case AST_INT: return asm_f_int(ast);
        case AST_FUNCTION: return asm_f_function(ast);
        case AST_STATEMENT: return asm_f_statement(ast);
        case AST_NOOP: return strdup("; No operation");
        case AST_DEFINITION_TYPE: return strdup("; Type definition");
        default: {
            char* s = calloc(64, sizeof(char));
            sprintf(s, "; Unknown AST type: %d", ast->type);
            return s;
        }
    }
}

#endif // SKULL_ASM_H_IMPLEMENTATION
#endif // SKULL_ASM_H
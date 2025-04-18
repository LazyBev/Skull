#ifndef SKULL_AST_H
#define SKULL_AST_H

#define SKULL_LIST_H_IMPLEMENTATION
#include "list.h"

typedef struct astStruct {
    enum {
        AST_COMPOUND,
        AST_FUNCTION,
        AST_CALL,
        AST_DEFINITION_TYPE,
        AST_VARIABLE,
        AST_STATEMENT,
        AST_INT,
        AST_NOOP,
        AST_ASSIGNMENT,
    } type;

    list_t* children;
    char* name;
    struct astStruct* value;
    int int_value;
    int data_type;
} ast_t;

ast_t* init_ast(int type);

#ifdef SKULL_AST_H_IMPLEMENTATION

ast_t* init_ast(int type) {
    ast_t* ast = calloc(1, sizeof(struct astStruct));
    ast->type = type;

    if (type == AST_COMPOUND) {
        ast->children = init_list(sizeof(struct astStruct));
    }

    return ast;
}

#endif // SKULL_AST_H_IMPLEMENTATION
#endif // SKULL_AST_H
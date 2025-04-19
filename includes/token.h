#ifndef SKULL_TOKEN_H
#define SKULL_TOKEN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_EXTRN,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_ASSIGN,
    TOKEN_SEMI,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_GT,
    TOKEN_EQ,
    TOKEN_LT,
    TOKEN_GTE,
    TOKEN_LTE,
    TOKEN_BANG,
    TOKEN_NEQ,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_DIVIDE,
    TOKEN_MULTIPLY,
    TOKEN_MODULUS,
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_MUTABLE,
    TOKEN_FUNC_TYPE,
} tokenType;

typedef struct tokenStruct {
    char *value;              
    tokenType type;
    unsigned int line;    // Track line number for error reporting
    unsigned int column;  // Track column number for error reporting
} token_t;

token_t *init_token(char *value, int type);
const char* token_type_to_str(int type);
char* token_to_str(token_t* token);
void free_token(token_t* token);

#ifdef SKULL_TOKEN_H_IMPLEMENTATION

token_t *init_token(char *value, int type) {
    token_t *token = calloc(1, sizeof(struct tokenStruct));
    if (!token) {
        fprintf(stderr, "Memory allocation failed for token\n");
        return NULL;
    }
    token->value = value;
    token->type = type;
    token->line = 0;   // Default values
    token->column = 0; // Will be set by lexer
    return token;
}

void free_token(token_t* token) {
    if (token) {
        if (token->value) {
            free(token->value);
        }
        free(token);
    }
}

const char* token_type_to_str(int type) {
    switch(type) {
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_ID: return "TOKEN_ID";
        case TOKEN_EXTRN: return "TOKEN_EXTRN";
        case TOKEN_LPAREN: return "TOKEN_LPAREN";
        case TOKEN_RPAREN: return "TOKEN_RPAREN";
        case TOKEN_LBRACE: return "TOKEN_LBRACE";
        case TOKEN_RBRACE: return "TOKEN_RBRACE";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_SEMI: return "TOKEN_SEMI";
        case TOKEN_COLON: return "TOKEN_COLON";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_GT: return "TOKEN_GT";
        case TOKEN_EQ: return "TOKEN_EQ";
        case TOKEN_LT: return "TOKEN_LT";
        case TOKEN_GTE: return "TOKEN_GTE";
        case TOKEN_LTE: return "TOKEN_LTE";
        case TOKEN_BANG: return "TOKEN_BANG";
        case TOKEN_NEQ: return "TOKEN_NEQ";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_DIVIDE: return "TOKEN_DIVIDE";
        case TOKEN_MULTIPLY: return "TOKEN_MULTIPLY";
        case TOKEN_MODULUS: return "TOKEN_MODULUS";
        case TOKEN_INT: return "TOKEN_INT";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_MUTABLE: return "TOKEN_MUTABLE";
        case TOKEN_FUNC_TYPE: return "TOKEN_FUNC_TYPE";
    }

    return "UNKNOWN_TOKEN_TYPE";
}

char* token_to_str(token_t* token) {
    if (!token) {
        return strdup("NULL_TOKEN");
    }
    
    const char* type_str = token_type_to_str(token->type);
    const char* template = "<type=\"%s\", int_type=%d, value='%s', line=%u, col=%u>";
    
    size_t value_len = token->value ? strlen(token->value) : 4; // 4 for "null"
    size_t type_len = strlen(type_str);
    
    // Allocate enough memory for the formatted string
    char* str = calloc(type_len + value_len + strlen(template) + 64, sizeof(char));
    if (!str) {
        fprintf(stderr, "Memory allocation failed in token_to_str\n");
        return NULL;
    }
    
    sprintf(str, template, type_str, token->type, 
            token->value ? token->value : "null", 
            token->line, token->column);

    return str;
}

#endif // SKULL_TOKEN_H_IMPLEMENTATION
#endif // SKULL_TOKEN_H
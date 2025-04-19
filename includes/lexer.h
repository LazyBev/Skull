#ifndef SKULL_LEXER_H
#define SKULL_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "utils.h"

typedef struct lexerStruct {
    char *src;
    size_t src_size;
    char c;
    unsigned int i;
    unsigned int line;
    unsigned int column;
} lexer_t;

lexer_t *init_lexer(char *src);
void lexer_advance(lexer_t* lexer);
void lexer_skip_whitespace(lexer_t* lexer);
token_t* lexer_advance_current(lexer_t* lexer, int type);
token_t* lexer_parse_id(lexer_t* lexer);
token_t* lexer_parse_number(lexer_t* lexer);
token_t* lexer_advance_with(lexer_t* lexer, token_t* token);
char lexer_peek(lexer_t* lexer, int offset);
token_t* lexer_next_token(lexer_t* lexer);
void lexer_error(lexer_t* lexer, const char* message);

#ifdef SKULL_LEXER_H_IMPLEMENTATION

lexer_t *init_lexer(char *src) {
    lexer_t *lexer = calloc(1, sizeof(struct lexerStruct));
    if (!lexer) {
        fprintf(stderr, "Memory allocation failed for lexer\n");
        exit(1);
    }
    lexer->src = src;
    lexer->src_size = strlen(src);
    lexer->i = 0;
    lexer->c = src[lexer->i];
    lexer->line = 1;    // Start at line 1
    lexer->column = 1;  // Start at column 1
    return lexer;
}

void lexer_advance(lexer_t* lexer) {
    if (lexer->i < lexer->src_size && lexer->c != '\0') {
        // Update line and column counters
        if (lexer->c == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        
        lexer->i += 1;
        lexer->c = lexer->src[lexer->i];
    }
}

void lexer_skip_whitespace(lexer_t* lexer) {
    while (lexer->c == '\r' || lexer->c == '\n' || lexer->c == ' ' || lexer->c == '\t') {
        lexer_advance(lexer);
    }
}

token_t* lexer_advance_current(lexer_t* lexer, int type) {
    char* value = calloc(2, sizeof(char));
    if (!value) {
        lexer_error(lexer, "Memory allocation failed");
        return NULL;
    }
    value[0] = lexer->c;
    value[1] = '\0';

    token_t* token = init_token(value, type);
    if (!token) {
        free(value);
        lexer_error(lexer, "Failed to create token");
        return NULL;
    }
    
    // Set token line and column info
    token->line = lexer->line;
    token->column = lexer->column;
    
    lexer_advance(lexer);
    return token;
}

token_t* lexer_parse_id(lexer_t* lexer) {
    unsigned int start_column = lexer->column;
    unsigned int start_line = lexer->line;
    
    char* value = calloc(1, sizeof(char));
    if (!value) {
        lexer_error(lexer, "Memory allocation failed");
        return NULL;
    }
    
    while (isalpha(lexer->c) || lexer->c == '_' || (strlen(value) > 0 && isdigit(lexer->c))) {
        size_t current_len = strlen(value);
        value = realloc(value, (current_len + 2) * sizeof(char));
        if (!value) {
            lexer_error(lexer, "Memory allocation failed");
            return NULL;
        }
        value[current_len] = lexer->c;
        value[current_len + 1] = '\0';
        lexer_advance(lexer);
    }
    
    token_t* token = init_token(value, TOKEN_ID);
    if (!token) {
        free(value);
        lexer_error(lexer, "Failed to create token");
        return NULL;
    }
    
    // Set token line and column info
    token->line = start_line;
    token->column = start_column;
    
    return token;
}

token_t* lexer_parse_number(lexer_t* lexer) {
    unsigned int start_column = lexer->column;
    unsigned int start_line = lexer->line;
    
    char* value = calloc(1, sizeof(char));
    if (!value) {
        lexer_error(lexer, "Memory allocation failed");
        return NULL;
    }
    
    while (isdigit(lexer->c)) {
        size_t current_len = strlen(value);
        value = realloc(value, (current_len + 2) * sizeof(char));
        if (!value) {
            lexer_error(lexer, "Memory allocation failed");
            return NULL;
        }
        value[current_len] = lexer->c;
        value[current_len + 1] = '\0';
        lexer_advance(lexer);
    }
    
    token_t* token = init_token(value, TOKEN_INT);
    if (!token) {
        free(value);
        lexer_error(lexer, "Failed to create token");
        return NULL;
    }
    
    // Set token line and column info
    token->line = start_line;
    token->column = start_column;
    
    return token;
}

token_t* lexer_advance_with(lexer_t* lexer, token_t* token) {
    lexer_advance(lexer);
    return token;
}

char lexer_peek(lexer_t* lexer, int offset) {
    size_t target_index = MIN(lexer->i + offset, lexer->src_size);
    return lexer->src[target_index];
}

void lexer_error(lexer_t* lexer, const char* message) {
    fprintf(stderr, "Lexer error at line %u, column %u: %s\n",
            lexer->line, lexer->column, message);
    fprintf(stderr, "Unexpected character: '%c'\n", lexer->c);
    exit(1);
}

token_t* lexer_next_token(lexer_t* lexer) {
    while (lexer->c != '\0') {
        lexer_skip_whitespace(lexer);
        
        // Skip comments
        if (lexer->c == '/' && lexer_peek(lexer, 1) == '/') {
            while (lexer->c != '\0' && lexer->c != '\n') {
                lexer_advance(lexer);
            }
            continue;
        }
        
        if (isalpha(lexer->c) || lexer->c == '_') 
            return lexer_parse_id(lexer);
        
        if (isdigit(lexer->c)) 
            return lexer_parse_number(lexer);

        switch (lexer->c) {
            case '=': {
                if (lexer_peek(lexer, 1) == '=') {
                    lexer_advance(lexer);
                    return lexer_advance_with(lexer, init_token("==", TOKEN_EQ));
                }
                return lexer_advance_current(lexer, TOKEN_ASSIGN);
            }
            case '!': {
                if (lexer_peek(lexer, 1) == '=') {
                    lexer_advance(lexer);
                    return lexer_advance_with(lexer, init_token("!=", TOKEN_NEQ));
                }
                return lexer_advance_current(lexer, TOKEN_BANG);
            }
            case '(': return lexer_advance_current(lexer, TOKEN_LPAREN);
            case ')': return lexer_advance_current(lexer, TOKEN_RPAREN);
            case '{': return lexer_advance_current(lexer, TOKEN_LBRACE);
            case '}': return lexer_advance_current(lexer, TOKEN_RBRACE);
            case ':': return lexer_advance_current(lexer, TOKEN_COLON);
            case ';': return lexer_advance_current(lexer, TOKEN_SEMI);
            case ',': return lexer_advance_current(lexer, TOKEN_COMMA);
            case '<': return lexer_advance_current(lexer, TOKEN_LT);
            case '>': return lexer_advance_current(lexer, TOKEN_GT);
            case '-': {
                if (lexer_peek(lexer, 1) == '>') {
                    unsigned int start_line = lexer->line;
                    unsigned int start_column = lexer->column;
                    token_t* token = init_token("->", TOKEN_FUNC_TYPE);
                    token->line = start_line;
                    token->column = start_column;
                    lexer_advance(lexer);
                    return lexer_advance_with(lexer, token);
                }
                return lexer_advance_current(lexer, TOKEN_MINUS);
            }
            case '+': return lexer_advance_current(lexer, TOKEN_PLUS);
            case '/': return lexer_advance_current(lexer, TOKEN_DIVIDE);
            case '*': return lexer_advance_current(lexer, TOKEN_MULTIPLY);
            case '%': return lexer_advance_current(lexer, TOKEN_MODULUS);
            case '\0': break;
            default: 
                lexer_error(lexer, "Unexpected token");
                break;
        }
    }

    token_t* eof_token = init_token(NULL, TOKEN_EOF);
    if (eof_token) {
        eof_token->line = lexer->line;
        eof_token->column = lexer->column;
    }
    return eof_token;
}

#endif // SKULL_LEXER_H_IMPLEMENTATION
#endif // SKULL_LEXER_H
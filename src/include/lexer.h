#ifndef SKULL_LEXER_H
#define SKULL_LEXER_H

#define SKULL_TOKEN_H_IMPLEMENTATION
#define SKULL_UTILS_H_IMPLEMENTATION
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

#ifdef SKULL_LEXER_H_IMPLEMENTATION

lexer_t *init_lexer(char *src) {
    lexer_t *lexer = calloc(1, sizeof(struct lexerStruct));
    lexer->src = src;
    lexer->src_size = strlen(src);
    lexer->i = 0;
    lexer->c = src[lexer->i];
    return lexer;
}

void lexer_advance(lexer_t* lexer) {
    if (lexer->i < lexer->src_size && lexer->c != '\0') {
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
    value[0] = lexer->c;
    value[1] = '\0';

    token_t* token = init_token(value, type);
    lexer_advance(lexer);

    return token;
}

token_t* lexer_parse_id(lexer_t* lexer) {
    char* value = calloc(1, sizeof(char));
    while (isalpha(lexer->c)) {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexer_advance(lexer);
    }
    return init_token(value, TOKEN_ID);
}

token_t* lexer_parse_number(lexer_t* lexer) {
    char* value = calloc(1, sizeof(char));
    while (isdigit(lexer->c)) {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexer_advance(lexer);
    }
    return init_token(value, TOKEN_INT);
}

token_t* lexer_advance_with(lexer_t* lexer, token_t* token) {
    lexer_advance(lexer);
    return token;
}

char lexer_peek(lexer_t* lexer, int offset) {
    return lexer->src[MIN(lexer->i + offset, lexer->src_size)];
}

token_t* lexer_next_token(lexer_t* lexer) {
    while (lexer->c != '\0') {
        lexer_skip_whitespace(lexer);
        
        if (isalpha(lexer->c)) 
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
                if (lexer_peek(lexer, 1) == '>') return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token("->", TOKEN_FUNC_TYPE)));
                return lexer_advance_current(lexer, TOKEN_MINUS);
            }
            case '+': return lexer_advance_current(lexer, TOKEN_PLUS);
            case '/': return lexer_advance_current(lexer, TOKEN_DIVIDE);
            case '*': return lexer_advance_current(lexer, TOKEN_MULTIPLY);
            case '%': return lexer_advance_current(lexer, TOKEN_MODULUS);
            case '\0': break;
            default: 
                printf("ERROR: Lexer found unexpected token: '%c'\n", lexer->c); 
                exit(1); 
                break;
        }
    }

    return init_token(0, TOKEN_EOF);
}

#endif // SKULL_LEXER_H_IMPLEMENTATION
#endif // SKULL_LEXER_H
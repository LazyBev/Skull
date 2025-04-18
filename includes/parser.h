#ifndef SKULL_PARSER_H
#define SKULL_PARSER_H

#define SKULL_LEXER_H_IMPLEMENTATION
#define SKULL_AST_H_IMPLEMENTATION
#define SKULL_TOKEN_H_IMPLEMENTATION
#define SKULL_TYPES_H_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "token.h"
#include "types.h"

typedef struct parserStruct {
    lexer_t* lexer;
    token_t* token;
} parser_t;

parser_t* init_parser(lexer_t* lexer);
ast_t* parse(parser_t* parser);
token_t* parser_eat(parser_t* parser, int type);
ast_t* parse_id(parser_t* parser);
ast_t* parse_block(parser_t* parser);
ast_t* parse_expr(parser_t* parser);
ast_t* parse_list(parser_t* parser);
ast_t* parse_compound(parser_t* parser);

#ifdef SKULL_PARSER_H_IMPLEMENTATION

parser_t* init_parser(lexer_t* lexer) {
    parser_t* parser = calloc(1, sizeof(struct parserStruct));
    parser->lexer = lexer;
    parser->token = lexer_next_token(lexer);

    return parser;
}

ast_t* parse(parser_t* parser) {
    return parse_compound(parser);
}

token_t* parser_eat(parser_t* parser, int type) {
    if (parser->token->type != type) {
        printf("ERROR: Parser found unexpected token: %s, was expecting: %s\n", token_to_str(parser->token), token_type_to_str(type)); 
        exit(1); 
    }

    parser->token = lexer_next_token(parser->lexer);
    return parser->token;
}

ast_t* parse_id(parser_t* parser) {
    char* value = calloc(strlen(parser->token->value) + 1, sizeof(char));
    strcpy(value, parser->token->value);
    parser_eat(parser, TOKEN_ID);

    if (parser->token->type == TOKEN_ASSIGN) {
        parser_eat(parser, TOKEN_ASSIGN);
        ast_t* ast = init_ast(AST_ASSIGNMENT);
        ast->name = value;
        ast->value = parse_expr(parser);
        return ast;
    }

    ast_t* ast = init_ast(AST_VARIABLE);
    ast->name = value;

    if (parser->token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);

        while (parser->token->type == TOKEN_ID) {
            ast->data_type = typename_to_int(parser->token->value);
            parser_eat(parser, TOKEN_ID);
            
            if (parser->token->type == TOKEN_LT) {
                parser_eat(parser, TOKEN_LT);
                ast->data_type += typename_to_int(parser->token->value);
                parser_eat(parser, TOKEN_ID);
                parser_eat(parser, TOKEN_GT);
            }
        }
    } else {
        if (parser->token->type == TOKEN_LPAREN) {
            ast->type = AST_CALL;
            parser_eat(parser, TOKEN_LPAREN);
            
            ast_t* args = init_ast(AST_COMPOUND);
            
            if (parser->token->type != TOKEN_RPAREN) {
                list_push(args->children, parse_expr(parser));
                
                while (parser->token->type == TOKEN_COMMA) {
                    parser_eat(parser, TOKEN_COMMA);
                    list_push(args->children, parse_expr(parser));
                }
            }
            
            parser_eat(parser, TOKEN_RPAREN);
            ast->value = args;
        }
    }

    return ast;
}

ast_t* parse_block(parser_t* parser) {
    parser_eat(parser, TOKEN_LBRACE);
    ast_t* ast = init_ast(AST_COMPOUND);

    while (parser->token->type != TOKEN_RBRACE) {
        list_push(ast->children, parse_expr(parser));
        
        if (parser->token->type == TOKEN_SEMI) {
            parser_eat(parser, TOKEN_SEMI);
        }
    }

    parser_eat(parser, TOKEN_RBRACE);
    return ast;
}

ast_t* parse_int(parser_t* parser) {
    int int_value = atoi(parser->token->value);
    parser_eat(parser, TOKEN_INT);

    ast_t* ast = init_ast(AST_INT);
    ast->int_value = int_value;

    return ast;
}

ast_t* parse_expr(parser_t* parser) {
    switch (parser->token->type) {
        case TOKEN_ID: {
            if (strcmp(parser->token->value, "return") == 0) {
                parser_eat(parser, TOKEN_ID);
                ast_t* ast = init_ast(AST_CALL);
                ast->name = "return";
                
                if (parser->token->type == TOKEN_LPAREN) {
                    parser_eat(parser, TOKEN_LPAREN);
                    ast->value = parse_expr(parser);
                    parser_eat(parser, TOKEN_RPAREN);
                }
                
                return ast;
            }
            return parse_id(parser);
        }
        case TOKEN_LPAREN: return parse_list(parser);
        case TOKEN_INT: return parse_int(parser);
        default: {printf("ERROR: Parser found unexpected token: %s\n", token_to_str(parser->token)); exit(1);};
    }
}

ast_t* parse_list(parser_t* parser) {
    parser_eat(parser, TOKEN_LPAREN);
    ast_t* ast = init_ast(AST_COMPOUND);
    
    if (parser->token->type != TOKEN_RPAREN) {
        list_push(ast->children, parse_expr(parser));

        while (parser->token->type == TOKEN_COMMA) {
            parser_eat(parser, TOKEN_COMMA);
            list_push(ast->children, parse_expr(parser));
        }
    }

    parser_eat(parser, TOKEN_RPAREN);

    if (parser->token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);

        while (parser->token->type == TOKEN_ID) {
            ast->data_type = typename_to_int(parser->token->value);
            parser_eat(parser, TOKEN_ID);
            if (parser->token->type == TOKEN_LT) {
                parser_eat(parser, TOKEN_LT);
                ast->data_type += typename_to_int(parser->token->value);
                parser_eat(parser, TOKEN_ID);
                parser_eat(parser, TOKEN_GT);
            }
        }
    }

    if (parser->token->type == TOKEN_FUNC_TYPE) {
        ast->type = AST_FUNCTION;
        parser_eat(parser, TOKEN_FUNC_TYPE);
        
        if (parser->token->type == TOKEN_ID) {
            ast->data_type = typename_to_int(parser->token->value);
            parser_eat(parser, TOKEN_ID);
        }
        
        ast->value = parse_block(parser);
    }

    return ast;
}

ast_t* parse_compound(parser_t* parser) {
    unsigned int should_close = 0;
    if (parser->token->type == TOKEN_LBRACE) {
        parser_eat(parser, TOKEN_LBRACE);
        should_close = 1;
    }
    ast_t* compound = init_ast(AST_COMPOUND);

    while (parser->token->type != TOKEN_EOF && parser->token->type != TOKEN_RBRACE) {
        list_push(compound->children, parse_expr(parser));

        if (parser->token->type == TOKEN_SEMI) {
            parser_eat(parser, TOKEN_SEMI);
        }
    }

    if (should_close) {
        parser_eat(parser, TOKEN_RBRACE);
    }

    return compound;
}

#endif // SKULL_PARSER_H_IMPLEMENTATION
#endif // SKULL_PARSER_H
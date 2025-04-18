#ifndef SKULL_PARSER_H
#define SKULL_PARSER_H

#define SKULL_LEXER_H_IMPLEMENTATION
#define SKULL_AST_H_IMPLEMENTATION
#define SKULL_TOKEN_H_IMPLEMENTATION
#define SKULL_TYPES_H_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // Added for boolean support
#include "lexer.h"
#include "ast.h"
#include "token.h"
#include "types.h"

/**
 * Parser structure with improved tracking of token history
 * for better error reporting and recovery options
 */
typedef struct parserStruct {
    lexer_t* lexer;
    token_t* token;         // Current token
    token_t* prev_token;    // Previous token for error context
    bool error_recovery;    // Flag for error recovery mode
    unsigned int error_count; // Count of errors encountered
} parser_t;

// Constructor/destructor functions
parser_t* init_parser(lexer_t* lexer);
void free_parser(parser_t* parser);

// Core parsing functions
ast_t* parse(parser_t* parser);
token_t* parser_eat(parser_t* parser, int type);
bool parser_expect(parser_t* parser, int type); // New helper for conditional checking

// Parse specific constructs
ast_t* parse_id(parser_t* parser);
ast_t* parse_block(parser_t* parser);
ast_t* parse_expr(parser_t* parser);
ast_t* parse_list(parser_t* parser);
ast_t* parse_compound(parser_t* parser);
ast_t* parse_function_declaration(parser_t* parser, char* name);
ast_t* parse_args_list(parser_t* parser);
ast_t* parse_int(parser_t* parser);

// Error handling
void parser_error(parser_t* parser, const char* message);
void parser_warning(parser_t* parser, const char* message); // Added warning function
bool parser_recover(parser_t* parser, int sync_token); // Error recovery function

#ifdef SKULL_PARSER_H_IMPLEMENTATION

parser_t* init_parser(lexer_t* lexer) {
    if (!lexer) {
        fprintf(stderr, "Error: NULL lexer provided to parser\n");
        return NULL;
    }

    parser_t* parser = calloc(1, sizeof(struct parserStruct));
    if (!parser) {
        fprintf(stderr, "Memory allocation failed for parser\n");
        exit(1);
    }
    
    parser->lexer = lexer;
    parser->token = lexer_next_token(lexer);
    parser->prev_token = NULL;
    parser->error_recovery = false;
    parser->error_count = 0;
    
    return parser;
}

void free_parser(parser_t* parser) {
    if (parser) {
        if (parser->prev_token) {
            free_token(parser->prev_token);
        }
        // Current token will be freed by lexer or other parts of the system
        free(parser);
    }
}

void parser_error(parser_t* parser, const char* message) {
    parser->error_count++;
    
    fprintf(stderr, "Parser error at line %u, column %u: %s\n", 
            parser->token ? parser->token->line : 0,
            parser->token ? parser->token->column : 0,
            message);
    
    if (parser->token) {
        char* token_str = token_to_str(parser->token);
        fprintf(stderr, "Current token: %s\n", token_str);
        free(token_str);
    }
    
    if (parser->prev_token) {
        char* prev_token_str = token_to_str(parser->prev_token);
        fprintf(stderr, "Previous token: %s\n", prev_token_str);
        free(prev_token_str);
    }
    
    // Only exit on first error or if error limit reached
    if (parser->error_count >= 5) {
        fprintf(stderr, "Too many parser errors, aborting compilation\n");
        exit(1);
    }
    
    // Set error recovery mode - further processing may continue
    parser->error_recovery = true;
}

void parser_warning(parser_t* parser, const char* message) {
    fprintf(stderr, "Parser warning at line %u, column %u: %s\n", 
            parser->token ? parser->token->line : 0,
            parser->token ? parser->token->column : 0,
            message);
}

bool parser_recover(parser_t* parser, int sync_token) {
    // Skip tokens until we reach the synchronization token
    while (parser->token && parser->token->type != TOKEN_EOF && 
           parser->token->type != sync_token) {
        parser_eat(parser, parser->token->type);
    }
    
    if (parser->token && parser->token->type == sync_token) {
        parser_eat(parser, sync_token);
        parser->error_recovery = false;
        return true;
    }
    
    return false;
}

bool parser_expect(parser_t* parser, int type) {
    if (parser->token && parser->token->type == type) {
        parser_eat(parser, type);
        return true;
    }
    
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), 
            "Expected token type %s, got %s", 
            token_type_to_str(type),
            parser->token ? token_type_to_str(parser->token->type) : "NULL");
    parser_error(parser, error_msg);
    return false;
}

ast_t* parse(parser_t* parser) {
    if (!parser) return NULL;
    return parse_compound(parser);
}

token_t* parser_eat(parser_t* parser, int type) {
    if (!parser->token) {
        parser_error(parser, "Unexpected end of input");
        return NULL;
    }
    
    if (parser->token->type != type && !parser->error_recovery) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "Expected token type %s, got %s", 
                token_type_to_str(type),
                token_type_to_str(parser->token->type));
        parser_error(parser, error_msg);
    }

    // Store previous token before getting the next one
    if (parser->prev_token) {
        free_token(parser->prev_token);
    }
    parser->prev_token = parser->token;
    
    parser->token = lexer_next_token(parser->lexer);
    return parser->token;
}

ast_t* parse_function_declaration(parser_t* parser, char* name) {
    if (!name) {
        parser_error(parser, "Function declaration missing name");
        return init_ast(AST_NOOP);
    }
    
    ast_t* ast = init_ast(AST_FUNCTION);
    if (!ast) {
        free(name); // Clean up the name if we can't create the AST
        parser_error(parser, "Failed to create AST node");
        return NULL;
    }
    
    ast->name = name;
    
    // Parse argument list
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        free_ast(ast); // Clean up on error
        return init_ast(AST_NOOP);
    }
    
    ast_t* args = init_ast(AST_COMPOUND);
    if (!args) {
        free_ast(ast);
        parser_error(parser, "Failed to create AST node for arguments");
        return NULL;
    }
    
    // Handle arguments
    if (parser->token && parser->token->type != TOKEN_RPAREN) {
        // Parse first argument
        if (parser->token->type == TOKEN_ID) {
            ast_t* arg = init_ast(AST_VARIABLE);
            if (!arg) {
                free_ast(args);
                free_ast(ast);
                parser_error(parser, "Failed to create AST node for argument");
                return NULL;
            }
            
            arg->name = strdup(parser->token->value);
            if (!arg->name) {
                free_ast(arg);
                free_ast(args);
                free_ast(ast);
                parser_error(parser, "Memory allocation failed");
                return NULL;
            }
            
            parser_eat(parser, TOKEN_ID);
            
            // Check for type annotation
            if (parser->token && parser->token->type == TOKEN_COLON) {
                parser_eat(parser, TOKEN_COLON);
                if (parser->token && parser->token->type == TOKEN_ID) {
                    arg->data_type = typename_to_int(parser->token->value);
                    parser_eat(parser, TOKEN_ID);
                    
                    // Handle generic types like Array<string>
                    if (parser->token && parser->token->type == TOKEN_LT) {
                        parser_eat(parser, TOKEN_LT);
                        // Just skip the inner type for now
                        if (parser->token && parser->token->type == TOKEN_ID) {
                            parser_eat(parser, TOKEN_ID);
                            if (parser->token && parser->token->type == TOKEN_GT) {
                                parser_eat(parser, TOKEN_GT);
                            }
                        }
                    }
                }
            }
            
            list_push(args->children, arg);
            
            // Parse additional arguments
            while (parser->token && parser->token->type == TOKEN_COMMA) {
                parser_eat(parser, TOKEN_COMMA);
                
                if (parser->token && parser->token->type == TOKEN_ID) {
                    arg = init_ast(AST_VARIABLE);
                    if (!arg) {
                        free_ast(args);
                        free_ast(ast);
                        parser_error(parser, "Failed to create AST node for argument");
                        return NULL;
                    }
                    
                    arg->name = strdup(parser->token->value);
                    if (!arg->name) {
                        free_ast(arg);
                        free_ast(args);
                        free_ast(ast);
                        parser_error(parser, "Memory allocation failed");
                        return NULL;
                    }
                    
                    parser_eat(parser, TOKEN_ID);
                    
                    if (parser->token && parser->token->type == TOKEN_COLON) {
                        parser_eat(parser, TOKEN_COLON);
                        if (parser->token && parser->token->type == TOKEN_ID) {
                            arg->data_type = typename_to_int(parser->token->value);
                            parser_eat(parser, TOKEN_ID);
                            
                            // Handle generic types
                            if (parser->token && parser->token->type == TOKEN_LT) {
                                parser_eat(parser, TOKEN_LT);
                                if (parser->token && parser->token->type == TOKEN_ID) {
                                    parser_eat(parser, TOKEN_ID);
                                    if (parser->token && parser->token->type == TOKEN_GT) {
                                        parser_eat(parser, TOKEN_GT);
                                    }
                                }
                            }
                        }
                    }
                    
                    list_push(args->children, arg);
                }
            }
        }
    }
    
    // Attach args to function
    ast->value = args;
    
    if (!parser_expect(parser, TOKEN_RPAREN)) {
        free_ast(ast); // Clean up on error
        return init_ast(AST_NOOP);
    }
    
    // Parse return type if specified
    if (parser->token && parser->token->type == TOKEN_FUNC_TYPE) {
        parser_eat(parser, TOKEN_FUNC_TYPE);
        
        if (parser->token && parser->token->type == TOKEN_ID) {
            ast->data_type = typename_to_int(parser->token->value);
            parser_eat(parser, TOKEN_ID);
        }
    }
    
    // Parse function body
    if (parser->token && parser->token->type == TOKEN_LBRACE) {
        ast_t* body = parse_block(parser);
        if (body) {
            // Update the function's value to point to the body
            // (first detach the args list if it exists)
            ast_t* args_list = ast->value;
            ast->value = body;
            
            // Store args list in function's children
            if (!ast->children) {
                ast->children = init_list(sizeof(struct astStruct));
            }
            
            if (args_list) {
                list_push(ast->children, args_list);
            }
        }
    }
    
    return ast;
}

ast_t* parse_id(parser_t* parser) {
    if (!parser->token || !parser->token->value) {
        parser_error(parser, "Invalid token");
        return init_ast(AST_NOOP);
    }
    
    char* value = strdup(parser->token->value);
    if (!value) {
        parser_error(parser, "Memory allocation failed");
        return NULL;
    }
    
    parser_eat(parser, TOKEN_ID);
    
    // Check for function declaration
    if (parser->token && parser->token->type == TOKEN_LPAREN) {
        return parse_function_declaration(parser, value);
    }

    // Handle assignment
    if (parser->token && parser->token->type == TOKEN_ASSIGN) {
        parser_eat(parser, TOKEN_ASSIGN);
        ast_t* ast = init_ast(AST_ASSIGNMENT);
        if (!ast) {
            free(value);
            parser_error(parser, "Failed to create AST node");
            return NULL;
        }
        
        ast->name = value;
        ast->value = parse_expr(parser);
        return ast;
    }

    // Handle variable or function call
    ast_t* ast = init_ast(AST_VARIABLE);
    if (!ast) {
        free(value);
        parser_error(parser, "Failed to create AST node");
        return NULL;
    }
    
    ast->name = value;

    // Type annotation
    if (parser->token && parser->token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);

        while (parser->token && parser->token->type == TOKEN_ID) {
            ast->data_type = typename_to_int(parser->token->value);
            parser_eat(parser, TOKEN_ID);
            
            if (parser->token && parser->token->type == TOKEN_LT) {
                parser_eat(parser, TOKEN_LT);
                if (parser->token && parser->token->type == TOKEN_ID) {
                    ast->data_type += typename_to_int(parser->token->value);
                    parser_eat(parser, TOKEN_ID);
                    if (parser->token && parser->token->type == TOKEN_GT) {
                        parser_eat(parser, TOKEN_GT);
                    }
                }
            }
        }
    }

    return ast;
}

ast_t* parse_block(parser_t* parser) {
    if (!parser_expect(parser, TOKEN_LBRACE)) {
        return init_ast(AST_NOOP);
    }
    
    ast_t* ast = init_ast(AST_COMPOUND);
    if (!ast) {
        parser_error(parser, "Failed to create AST node for block");
        return NULL;
    }

    while (parser->token && parser->token->type != TOKEN_RBRACE && 
           parser->token->type != TOKEN_EOF) {
        ast_t* expr = parse_expr(parser);
        if (expr) {
            list_push(ast->children, expr);
        }
        
        if (parser->token && parser->token->type == TOKEN_SEMI) {
            parser_eat(parser, TOKEN_SEMI);
        } else if (parser->token && parser->token->type != TOKEN_RBRACE) {
            parser_warning(parser, "Missing semicolon");
        }
    }

    if (!parser_expect(parser, TOKEN_RBRACE)) {
        // Error recovery already handled in parser_expect
        // Still return the AST we've built so far
    }
    
    return ast;
}

ast_t* parse_int(parser_t* parser) {
    if (!parser->token || !parser->token->value) {
        parser_error(parser, "Invalid token for integer");
        return init_ast(AST_NOOP);
    }
    
    int int_value = atoi(parser->token->value);
    parser_eat(parser, TOKEN_INT);

    ast_t* ast = init_ast(AST_INT);
    if (!ast) {
        parser_error(parser, "Failed to create AST node for integer");
        return NULL;
    }
    
    ast->int_value = int_value;

    return ast;
}

ast_t* parse_expr(parser_t* parser) {
    if (!parser->token) {
        parser_error(parser, "Unexpected end of input");
        return init_ast(AST_NOOP);
    }
    
    switch (parser->token->type) {
        case TOKEN_ID: {
            if (parser->token->value && strcmp(parser->token->value, "return") == 0) {
                parser_eat(parser, TOKEN_ID);
                ast_t* ast = init_ast(AST_STATEMENT);
                if (!ast) {
                    parser_error(parser, "Failed to create AST node for return statement");
                    return NULL;
                }
                
                ast->name = strdup("return");
                if (!ast->name) {
                    free_ast(ast);
                    parser_error(parser, "Memory allocation failed");
                    return NULL;
                }
                
                if (parser->token && parser->token->type == TOKEN_LPAREN) {
                    parser_eat(parser, TOKEN_LPAREN);
                    ast->value = parse_expr(parser);
                    if (!parser_expect(parser, TOKEN_RPAREN)) {
                        free_ast(ast);
                        return init_ast(AST_NOOP);
                    }
                }
                
                return ast;
            }
            return parse_id(parser);
        }
        case TOKEN_LPAREN: return parse_list(parser);
        case TOKEN_INT: return parse_int(parser);
        case TOKEN_EOF: {
            parser_error(parser, "Unexpected end of file");
            return init_ast(AST_NOOP);
        }
        default: {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "Unexpected token type: %s", 
                    token_type_to_str(parser->token->type));
            parser_error(parser, error_msg);
            
            // Skip token and try to continue
            parser_eat(parser, parser->token->type);
            return init_ast(AST_NOOP);
        }
    }
}

ast_t* parse_list(parser_t* parser) {
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        return init_ast(AST_NOOP);
    }
    
    ast_t* ast = init_ast(AST_COMPOUND);
    if (!ast) {
        parser_error(parser, "Failed to create AST node for list");
        return NULL;
    }
    
    if (parser->token && parser->token->type != TOKEN_RPAREN) {
        ast_t* expr = parse_expr(parser);
        if (expr) {
            list_push(ast->children, expr);
        }

        while (parser->token && parser->token->type == TOKEN_COMMA) {
            parser_eat(parser, TOKEN_COMMA);
            expr = parse_expr(parser);
            if (expr) {
                list_push(ast->children, expr);
            }
        }
    }

    if (!parser_expect(parser, TOKEN_RPAREN)) {
        // Error handling already done in parser_expect
        // Still return what we've parsed
    }

    if (parser->token && parser->token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);

        while (parser->token && parser->token->type == TOKEN_ID) {
            ast->data_type = typename_to_int(parser->token->value);
            parser_eat(parser, TOKEN_ID);
            
            if (parser->token && parser->token->type == TOKEN_LT) {
                parser_eat(parser, TOKEN_LT);
                
                if (parser->token && parser->token->type == TOKEN_ID) {
                    ast->data_type += typename_to_int(parser->token->value);
                    parser_eat(parser, TOKEN_ID);
                    
                    if (parser->token && parser->token->type == TOKEN_GT) {
                        parser_eat(parser, TOKEN_GT);
                    }
                }
            }
        }
    }

    return ast;
}

ast_t* parse_compound(parser_t* parser) {
    ast_t* compound = init_ast(AST_COMPOUND);
    if (!compound) {
        parser_error(parser, "Failed to create AST node for compound statement");
        return NULL;
    }

    while (parser->token && parser->token->type != TOKEN_EOF && 
           parser->token->type != TOKEN_RBRACE) {
        ast_t* expr = parse_expr(parser);
        if (expr) {
            list_push(compound->children, expr);
        }

        if (parser->token && parser->token->type == TOKEN_SEMI) {
            parser_eat(parser, TOKEN_SEMI);
        } else if (parser->token && 
                   parser->token->type != TOKEN_RBRACE && 
                   parser->token->type != TOKEN_EOF) {
            parser_warning(parser, "Missing semicolon");
        }
    }

    return compound;
}

#endif // SKULL_PARSER_H_IMPLEMENTATION
#endif // SKULL_PARSER_H

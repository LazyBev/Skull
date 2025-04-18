// list.h
#ifndef SKULL_LIST_H
#define SKULL_LIST_H

#define SKULL_AST_H_IMPLEMENTATION
#define SKULL_PARSER_H_IMPLEMENTATION
#include <stdlib.h>
#include "ast.h"
#include "parser.h"

typedef struct {
    void** items;
    size_t size;
    size_t item_size;
} list_t;

list_t* init_list(size_t item_size);
void list_push(list_t* list, void* item);
void free_list(list_t* list);

#ifdef SKULL_LIST_H_IMPLEMENTATION

list_t* init_list(size_t item_size) {
    list_t* list = calloc(1, sizeof(list_t));
    if (!list) return NULL;
    
    list->items = NULL;
    list->size = 0;
    list->item_size = item_size;
    
    return list;
}

void list_push(list_t* list, void* item) {
    if (!list) return;
    
    list->size++;
    list->items = realloc(list->items, sizeof(void*) * list->size);
    
    if (!list->items) {
        fprintf(stderr, "Memory allocation failed in list_push\n");
        exit(1);
    }
    
    list->items[list->size - 1] = item;
}

void free_list(list_t* list) {
    if (!list) return;
    
    if (list->items) {
        free(list->items);
    }
    free(list);
}

#endif // SKULL_LIST_H_IMPLEMENTATION
#endif // SKULL_LIST_H
#ifndef SKULL_LIST_H
#define SKULL_LIST_H

typedef struct listStruct {
    void** items;
    size_t size;
    size_t item_size;
} list_t;

list_t* init_list(size_t item_size);

void list_push(list_t* list, void* item);

#ifdef SKULL_LIST_H_IMPLEMENTATION

list_t* init_list(size_t item_size) {
    list_t* list = calloc(1, sizeof(struct listStruct));
    list->size = 0;
    list->item_size = item_size;
    list->items = 0;

    return list;
}

void list_push(list_t* list, void* item) {
    list->size += 1;

    if (!list->items) {
        list->items = calloc(1, list->item_size);
    } else {
        list->items = realloc(list->items, (list->size  * list->item_size));
    }
    
    list->items[list->size-1] = item;
}

#endif // SKULL_LIST_H_IMPLEMENTATION
#endif // SKULL_LIST_H
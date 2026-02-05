#pragma once

#include "defines.h"

typedef struct Node
{
    void* data;
    struct Node* next;
} Node;

/**
 * @brief A linked list in which a new node is inserted at the front and only the first occurrence of existing node is deleted.
 */
typedef struct Linked_List
{
    char type[32];
    u32 data_size;
    Node** head;
} Linked_List;

Linked_List* linked_list_create(char const* type, u32 data_size);
void linked_list_destroy(Linked_List* list);
void linked_list_insert(Linked_List* list, void const* data, u32 data_size);
void linked_list_delete(Linked_List* list, void const* data, bool (* compare)(void const*, void const*));
bool linked_list_contains(Linked_List const* list, void const* data, bool (* compare)(void const*, void const*));

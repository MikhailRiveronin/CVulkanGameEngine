#pragma once

#include "defines.h"

typedef struct Linked_List_Node
{
    void* data;
    struct Linked_List_Node* next;
} Linked_List_Node;

typedef void* (* Linked_List_Insert_Callback)(void const*);
typedef void (* Linked_List_Erase_Callback)(void*);
typedef bool (* Linked_List_Compare_Callback)(void const*, void const*);

/**
 * @brief A linked list in which only the first occurrence of existing node is erased.
 * The linked list takes ownership of the data and is responsible for deleting the data.
 */
typedef struct Linked_List
{
    char type[32];
    Linked_List_Node** head;

    Linked_List_Insert_Callback insert;
    Linked_List_Erase_Callback erase;
    Linked_List_Compare_Callback compare;
} Linked_List;

LIB_API Linked_List* linked_list_create(char const* type, Linked_List_Insert_Callback insert, Linked_List_Erase_Callback erase, Linked_List_Compare_Callback compare);
LIB_API void linked_list_destroy(Linked_List* list);
LIB_API void linked_list_insert_front(Linked_List* list, void const* data);

/**
 * @brief Inserts new node. If prev is 0, insert in front.
 */
LIB_API void linked_list_insert_after(Linked_List* list, Linked_List_Node const* prev, void const* data);

LIB_API void linked_list_erase(Linked_List* list, Linked_List_Node const* node);
LIB_API Linked_List_Node* linked_list_find(Linked_List* list, void const* key);
LIB_API bool linked_list_contains(Linked_List const* list, void const* data);
LIB_API void* linked_list_at(Linked_List const* list, void const* data);
LIB_API void linked_list_sort(Linked_List* list);

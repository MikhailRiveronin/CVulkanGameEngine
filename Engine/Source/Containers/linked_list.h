#pragma once

#include "defines.h"

typedef struct Node
{
    void* data;
    struct Node* next;
} Node;

typedef void* (* Linked_List_Insert_Callback)(void const*, u32);
typedef void (* Linked_List_Erase_Callback)(void*, u32);
typedef bool (* Linked_List_Compare_Callback)(void const*, void const*, u32 data_size);

/**
 * @brief A linked list in which a new node is inserted at the front and only the first occurrence of existing node is deleted. The linked list takes ownership of the data and is responsible for deleting the data.
 */
typedef struct Linked_List
{
    char type[32];
    u32 data_size;
    Node** head;

    Linked_List_Insert_Callback insert;
    Linked_List_Erase_Callback erase;
    Linked_List_Compare_Callback compare;
} Linked_List;

Linked_List* linked_list_create(char const* type, u32 data_size, Linked_List_Insert_Callback insert, Linked_List_Erase_Callback erase, Linked_List_Compare_Callback compare);
void linked_list_destroy(Linked_List* list);
void linked_list_insert(Linked_List* list, void const* data);
void linked_list_erase(Linked_List* list, void const* data);
bool linked_list_contains(Linked_List const* list, void const* data);
void* linked_list_find(Linked_List const* list, void const* data);

#define LINKED_LIST_CREATE(type, insert, erase, compare) linked_list_create((#type), sizeof(type), (insert), (erase), (compare))

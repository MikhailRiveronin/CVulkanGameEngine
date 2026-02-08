#include "linked_list.h"

#include "core/logger.h"
#include "systems/memory_system.h"

static void free_list(Linked_List* list);
static Linked_List_Node* merge(Linked_List_Node* first, Linked_List_Node* second, Linked_List_Compare_Callback compare);
static Linked_List_Node* merge_sort(Linked_List_Node* head, Linked_List_Compare_Callback compare);

Linked_List* linked_list_create(char const* type, Linked_List_Insert_Callback insert, Linked_List_Erase_Callback erase, Linked_List_Compare_Callback compare)
{
    Linked_List* list = memory_system_allocate(sizeof(*list), MEMORY_TAG_CONTAINERS);
    strncpy(list->type, type, sizeof(list->type) - 1);
    list->head = memory_system_allocate(sizeof(*list->head), MEMORY_TAG_CONTAINERS);
    *list->head = 0;
    list->insert = insert;
    list->erase = erase;
    list->compare = compare;
    return list;
}

void linked_list_destroy(Linked_List* list)
{
    free_list(list);
    memory_system_free(list->head, sizeof(*list->head), MEMORY_TAG_CONTAINERS);
    memory_system_free(list, sizeof(*list), MEMORY_TAG_CONTAINERS);
}

void linked_list_insert_front(Linked_List* list, void const* data)
{
    Linked_List_Node* new_node = memory_system_allocate(sizeof(*new_node), MEMORY_TAG_CONTAINERS);
    new_node->data = list->insert(data);
    new_node->next = *list->head;
    *list->head = new_node;
}

void linked_list_insert_after(Linked_List* list, Linked_List_Node* prev, void const* data)
{
    Linked_List_Node* new_node = memory_system_allocate(sizeof(*new_node), MEMORY_TAG_CONTAINERS);
    new_node->data = list->insert(data);

    Linked_List_Node** head = list->head;
    if (prev)
    {
        new_node->next = prev->next;
        prev->next = new_node;
    }
    else
    {
        new_node->next = *list->head;
        *list->head = new_node;
    }
}

void linked_list_erase(Linked_List* list, Linked_List_Node const* node)
{
    for (Linked_List_Node** head = list->head; *head; head = &(*head)->next)
    {
        if (*head == node)
        {
            Linked_List_Node* next = (*head)->next;
            list->erase((*head)->data);
            memory_system_free(*head, sizeof(**head), MEMORY_TAG_CONTAINERS);
            *head = next;
            return;
        }
    }

    LOG_WARNING("linked_list_erase: Trying to delete non-existing data");
}

Linked_List_Node* linked_list_find(Linked_List* list, void const* key)
{
    for (Linked_List_Node** head = list->head; *head; head = &(*head)->next)
    { 
        if (list->compare((*head)->data, key))
        {
            return *head;
        }
    }

    return 0;
}

bool linked_list_contains(Linked_List const* list, void const* data)
{
    for (Linked_List_Node** head = list->head; *head; head = &(*head)->next)
    { 
        if (list->compare((*head)->data, data))
        {
            return true;
        }
    }

    return false;
}

void* linked_list_at(Linked_List const* list, void const* data)
{
    for (Linked_List_Node** head = list->head; *head; head = &(*head)->next)
    {
        if (list->compare((*head)->data, data))
        {
            return (*head)->data;
        }
    }

    LOG_WARNING("linked_list_at: Trying to access non-existing data");
    return 0;
}

void linked_list_sort(Linked_List* list)
{
    *list->head = merge_sort(*list->head, list->compare);
}

void free_list(Linked_List* list)
{
    while (*list->head)
    {
        Linked_List_Node* next = (*list->head)->next;
        list->erase((*list->head)->data);
        memory_system_free(*list->head, sizeof(**list->head), MEMORY_TAG_CONTAINERS);
        *list->head = next;
    }
}

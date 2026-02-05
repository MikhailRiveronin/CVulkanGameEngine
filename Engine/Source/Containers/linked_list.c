#include "linked_list.h"

#include "core/logger.h"
#include "core/string_utils.h"
#include "systems/memory_system.h"

static inline void free_list(Linked_List* list);

Linked_List* linked_list_create(char const* type, u32 data_size, Linked_List_Insert_Callback insert, Linked_List_Erase_Callback erase, Linked_List_Compare_Callback compare)
{
    Linked_List* list = memory_system_allocate(sizeof(*list), MEMORY_TAG_CONTAINERS);
    strncpy(list->type, type, sizeof(list->type) - 1);
    list->data_size = data_size;
    list->head = memory_system_allocate(sizeof(*list->head), MEMORY_TAG_CONTAINERS);
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

void linked_list_insert(Linked_List* list, void const* data)
{
    Node* new_node = memory_system_allocate(sizeof(*new_node), MEMORY_TAG_CONTAINERS);
    new_node->data = list->insert(data, list->data_size);
    new_node->next = *list->head;
    *list->head = new_node;
}

void linked_list_erase(Linked_List* list, void const* data)
{
    Node** head = list->head;
    for (; *head; head = &(*head)->next)
    {
        if (list->compare((*head)->data, data, list->data_size))
        {
            Node* next = (*head)->next;
            list->erase((*head)->data, list->data_size);
            memory_system_free(*head, sizeof(**head), MEMORY_TAG_CONTAINERS);
            *head = next;
            return;
        }
    }

    LOG_WARNING("linked_list_erase: Trying to delete non-existing data");
}

bool linked_list_contains(Linked_List const* list, void const* data)
{
    Node** head = list->head;
    for (; *head; head = &(*head)->next)
    {
        if (list->compare((*head)->data, data, list->data_size))
        {
            return true;
        }
    }

    return false;
}

void* linked_list_find(Linked_List const* list, void const* data)
{
    Node** head = list->head;
    for (; *head; head = &(*head)->next)
    {
        if (list->compare((*head)->data, data, list->data_size))
        {
            return (*head)->data;
        }
    }

    LOG_WARNING("linked_list_find: Trying to find non-existing data");
    return 0;
}

void free_list(Linked_List* list)
{
    while (*list->head)
    {
        Node* next = (*list->head)->next;
        list->erase((*list->head)->data, list->data_size);
        memory_system_free(*list->head, sizeof(**list->head), MEMORY_TAG_CONTAINERS);
        *list->head = next;
    }
}

#include "linked_list.h"

#include "core/logger.h"
#include "core/string_utils.h"
#include "systems/memory_system.h"

static void free_list(Linked_List* list);

Linked_List* linked_list_create(char const* type, u32 data_size)
{
    Linked_List* list = memory_system_allocate(sizeof(*list), MEMORY_TAG_CONTAINERS);
    strncpy(list->type, type, sizeof(list->type) - 1);
    list->data_size = data_size;
    list->head = memory_system_allocate(sizeof(*list->head), MEMORY_TAG_CONTAINERS);
    return list;
}

void linked_list_destroy(Linked_List* list)
{
    free_list(list);
    memory_system_free(list->head, sizeof(*list->head), MEMORY_TAG_CONTAINERS);
    memory_system_free(list, sizeof(*list), MEMORY_TAG_CONTAINERS);
}

void linked_list_insert(Linked_List* list, void const* data, u32 data_size)
{
    if ((*list->head)->next && data_size != list->data_size)
    {
        LOG_FATAL("linked_list_insert: Trying to insert data of different type. Not happening");
        return;
    }

    Node* new_node = memory_system_allocate(sizeof(*new_node), MEMORY_TAG_CONTAINERS);
    memory_system_copy(new_node->data, data, list->data_size);
    new_node->next = *list->head;
    *list->head = new_node;
}

void linked_list_delete(Linked_List* list, void const* data, bool (* compare)(void const*, void const*))
{
    Node** head = list->head;
    for (; *head; head = &(*head)->next)
    {
        if (compare((*head)->data, data))
        {
            Node* next = (*head)->next;
            memory_system_free((*head)->data, list->data_size, MEMORY_TAG_CONTAINERS);
            memory_system_free(*head, sizeof(**head), MEMORY_TAG_CONTAINERS);
            *head = next;
        }
    }

    LOG_WARNING("linked_list_delete: Trying to delete non-existing data");
}

bool linked_list_contains(Linked_List const* list, void const* data, bool (* compare)(void const*, void const*))
{
    Node** head = list->head;
    for (; *head; head = &(*head)->next)
    {
        if (compare((*head)->data, data))
        {
            return true;
        }
    }

    return false;
}

void free_list(Linked_List* list)
{
    while (*list->head)
    {
        Node* next = (*list->head)->next;
        memory_system_free((*list->head)->data, list->data_size, MEMORY_TAG_CONTAINERS);
        memory_system_free(*list->head, sizeof(**list->head), MEMORY_TAG_CONTAINERS);
        *list->head = next;
    }
}

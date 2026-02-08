#include "freelist.h"

#include "core/logger.h"
#include "systems/memory_system.h"

typedef struct Freelist_Entry
{
    u32 offset;
    u32 size;
} Freelist_Entry;

static void* insert(void const* data);
static void erase(void* data);
static bool compare(void const* node_data, void const* offset);
static void coalesce(Linked_List const* nodes, Linked_List_Node* first);

Freelist* freelist_create(u32 size)
{
    Freelist* list = memory_system_allocate(sizeof(*list), MEMORY_TAG_CONTAINERS);
    list->total_size = size;
    list->nodes = linked_list_create("Freelist_Entry", insert, erase, compare);

    Freelist_Entry entry;
    entry.offset = 0;
    entry.size = list->total_size;
    linked_list_insert_front(list->nodes, &entry);
    return list;
}

void freelist_destroy(Freelist* list)
{
    linked_list_destroy(list->nodes);
    memory_system_free(list, sizeof(*list), MEMORY_TAG_CONTAINERS);
}

bool freelist_allocate(Freelist* list, u32 required_size, u32* offset)
{
    Linked_List_Node** head = list->nodes->head;
    for (; *head; head = &(*head)->next)
    {
        Freelist_Entry* entry = (*head)->data;
        if (entry->size >= required_size)
        {
            *offset = entry->offset;
            entry->offset += required_size;
            entry->size -= required_size;
            if (entry->size == 0)
            {
                linked_list_erase(list->nodes, *head);
            }

            return true;
        }
    }

    LOG_WARNING("freelist_allocate: Failed to find block with required size");
    return false;
}

bool freelist_free(Freelist* list, u32 offset, u32 size)
{
    Freelist_Entry new_entry;
    new_entry.offset = offset;
    new_entry.size = size;

    Linked_List_Node* prev = 0;
    for (Linked_List_Node** head = list->nodes->head; (*head); head = &(*head)->next)
    {
        if (((Freelist_Entry*)*head)->offset > offset)
        {
            linked_list_insert_after(list->nodes, prev, &new_entry);
            coalesce(list->nodes, prev);
            return true;
        }

        prev = *head;
    }

    if ((new_entry.offset + new_entry.size) <= list->total_size)
    {
        linked_list_insert_after(list->nodes, prev, &new_entry);
        coalesce(list->nodes, prev);
        return true;
    }

    LOG_WARNING("freelist_free: Failed to free block at required offset");
    return false;
}

void* insert(void const* data)
{
    void* new_data = memory_system_allocate(sizeof(Freelist_Entry), MEMORY_TAG_CONTAINERS);
    memory_system_copy(new_data, data, sizeof(Freelist_Entry));
    return new_data;
}

void erase(void* data)
{
    memory_system_free(data, sizeof(Freelist_Entry), MEMORY_TAG_CONTAINERS);
}

bool compare(void const* node_data, void const* offset)
{
    return ((Freelist_Entry*)node_data)->offset == *(u32*)offset;
}

void coalesce(Linked_List const* nodes, Linked_List_Node* first)
{
    Linked_List_Node* second = first->next;
    Linked_List_Node* third = second->next;

    Freelist_Entry* first_entry = first->data;
    Freelist_Entry* second_entry = second->data;
    if (third)
    {
        Freelist_Entry* third_entry = third->data;
        if ((second_entry->offset + second_entry->size) == third_entry->offset)
        {
            second_entry->size += third_entry->size;
            linked_list_erase(nodes, third);
        }
    }

    if ((first_entry->offset + first_entry->size) == second_entry->offset)
    {
        first_entry->size += second_entry->size;
        linked_list_erase(nodes, second);
    }
}

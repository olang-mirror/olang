/*
 * Copyright (C) 2024 olang maintainers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "list.h"
#include <assert.h>

void
list_init(list_t *list, arena_t *arena)
{
    assert(list != NULL);
    list->size = 0;
    list->arena = arena;
    list->head = NULL;
}

void
list_append(list_t *list, void *value)
{
    assert(list != NULL);
    list_item_t *item = arena_alloc(list->arena, sizeof(list_item_t));
    item->value = value;
    item->next = NULL;
    list->size++;

    if (list->size == 1) {
        list->head = item;
        list->tail = item;
        return;
    }

    list->tail->next = item;
    list->tail = item;
}

list_item_t *
list_get(list_t *list, size_t index)
{
    assert(list != NULL);
    assert(index < list->size);

    list_item_t *item = list->head;

    while (index != 0) {
        item = item->next;

        index--;
    }

    return item;
}

list_item_t *
list_head(list_t *list)
{
    assert(list != NULL);
    return list->head;
}

list_item_t *
list_next(list_item_t *item)
{
    assert(item != NULL);
    return item->next;
}

size_t
list_size(list_t *list)
{
    assert(list != NULL);
    return list->size;
}

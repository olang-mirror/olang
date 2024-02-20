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
#ifndef LIST_H
#define LIST_H
#include "arena.h"

typedef struct list_item
{
    void *value;
    struct list_item *next;
} list_item_t;

typedef struct list
{
    size_t size;
    arena_t *arena;
    list_item_t *head;
    list_item_t *tail;
} list_t;

void
list_init(list_t *list, arena_t *arena);

void
list_append(list_t *list, void *value);

list_item_t *
list_get(list_t *list, size_t index);

list_item_t *
list_head(list_t *list);

list_item_t *
list_next(list_item_t *item);

size_t
list_size(list_t *list);
#endif

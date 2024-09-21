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
#ifndef MAP_H
#define MAP_H

#include "arena.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAP_INITIAL_CAPACITY 32

#define U32_FNV1A_PRIME 0x01000193
#define U32_FNV1A_OFFSET_BASIS 0x811c9dc5

typedef struct map map_t;
typedef struct map_bucket map_bucket_t;
typedef struct map_entry map_entry_t;

typedef struct map
{
    arena_t *arena;
    map_entry_t *entries;
    size_t capacity;
    size_t size;
} map_t;

typedef struct map_kv
{
    char *key;
    void *value;
} map_kv_t;

typedef struct map_entry
{
    char *key;
    void *value;
    uint32_t hash;
    map_entry_t *next;
} map_entry_t;

map_t *
map_new(arena_t *arena);

bool
map_put(map_t *map, char *key, void *value);

void *
map_get(map_t *map, char *key);

void
map_get_kvs(map_t *map, map_kv_t **kvs);

#endif /* MAP_H */

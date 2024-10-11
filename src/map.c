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
#include "map.h"
#include "arena.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t
u32_fnv1a_hash(const char *s);

static void
map_init(map_t *map);

static char *
_strdup(const char *s, arena_t *arena);

static uint32_t
map_get_index(map_t *map, uint32_t hash);

map_t *
map_new(arena_t *arena)
{
    map_t *map = (map_t *)arena_alloc(arena, sizeof(map_t));
    if (map == NULL) {
        fprintf(
            stderr, "[FATAL] Out of memory: map_new: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    map->arena = arena;
    map_init(map);
    return map;
}

static void
map_init(map_t *map)
{
    assert(map);
    map->entries = (map_entry_t *)arena_alloc(
        map->arena, MAP_INITIAL_CAPACITY * sizeof(map_entry_t));
    assert(map->entries != NULL);
    memset(map->entries, 0, MAP_INITIAL_CAPACITY * sizeof(map_entry_t));
    if (map->entries == NULL) {
        fprintf(
            stderr, "[FATAL] Out of memory: map_init: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    map->capacity = MAP_INITIAL_CAPACITY;
}

static uint32_t
u32_fnv1a_hash(const char *s)
{
    uint32_t hash = U32_FNV1A_OFFSET_BASIS;
    size_t len = strlen(s);
    for (size_t i = 0; i < len; ++i) {
        hash ^= s[i];
        hash *= U32_FNV1A_PRIME;
    }
    return hash;
}

bool
map_put(map_t *map, char *key, void *value)
{
    assert(map && key);
    map->size++;

    uint32_t hash = u32_fnv1a_hash(key);
    map_entry_t *entry = map->entries + map_get_index(map, hash);

    if (entry->key == NULL) {
        *entry = (map_entry_t){
            .key = _strdup(key, map->arena),
            .hash = hash,
            .value = value,
            .next = NULL,
        };
        return true;
    }

    do {
        if (entry->hash == hash && strcmp(entry->key, key) == 0) {
            entry->value = value;
            break;
        }
        if (entry->next == NULL) {
            entry->next =
                (map_entry_t *)arena_alloc(map->arena, sizeof(map_entry_t));
            *entry->next = (map_entry_t){
                .key = _strdup(key, map->arena),
                .hash = hash,
                .value = value,
                .next = NULL,
            };

            break;
        }
        entry = entry->next;
    } while (entry != NULL);

    return true;
}

void *
map_get(map_t *map, char *key)
{
    uint32_t hash = u32_fnv1a_hash(key);
    map_entry_t *entry = map->entries + map_get_index(map, hash);
    while (entry != NULL) {
        if (entry->hash == hash && strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

static uint32_t
map_get_index(map_t *map, uint32_t hash)
{
    uint32_t capacity_mask = map->capacity - 1;
    return hash & capacity_mask;
}

void
map_get_kvs(map_t *map, map_kv_t **kvs)
{
    size_t index = 0;

    for (size_t j = 0; j < map->capacity; ++j) {
        map_entry_t *entry = map->entries + j;

        while (entry != NULL && entry->key != NULL) {
            kvs[index++] = (map_kv_t *)entry;
            entry = entry->next;
        }
    }
}

static char *
_strdup(const char *s, arena_t *arena)
{
    size_t slen = strlen(s);
    char *result = arena_alloc(arena, slen + 1);
    if (result == NULL) {
        return NULL;
    }

    memcpy(result, s, slen + 1);
    return result;
}

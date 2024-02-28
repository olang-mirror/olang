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
#include "arena.h"
#include <stdio.h>
#include <stdlib.h>

arena_t
arena_new(size_t size)
{
    arena_t arena;
    arena.offset = 0;
    arena.region = malloc(sizeof(uint8_t) * size);
    arena.size = size;
    return arena;
}

static uint8_t
arena_padding(size_t bytes);

void *
arena_alloc(arena_t *arena, size_t bytes)
{
    if ((arena->offset + bytes) > arena->size) {
        return NULL;
    }
    void *pointer = arena->region + arena->offset;
    arena->offset += bytes + arena_padding(bytes);

    return pointer;
}

void
arena_release(arena_t *arena)
{
    arena->offset = 0;
}

void
arena_free(arena_t *arena)
{
    arena->size = 0;
    free(arena->region);
}

static uint8_t
arena_padding(size_t bytes)
{
    return (ARENA_ALIGNMENT_BYTES - bytes) & ARENA_ALIGNMENT_BYTES_MASK;
}

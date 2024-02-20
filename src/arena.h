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
#ifndef ARENA_H
#define ARENA_H
#include <stdint.h>
#include <stdlib.h>

typedef struct arena
{
    size_t offset;
    size_t size;
    uint8_t *region;
} arena_t;

arena_t
arena_new(size_t size);

void *
arena_alloc(arena_t *arena, size_t size);

void
arena_release(arena_t *arena);

void
arena_free(arena_t *arena);

#endif

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
#ifndef SCOPE_H
#define SCOPE_H

#include "arena.h"
#include "map.h"
#include "string_view.h"

typedef struct symbol
{
    string_view_t id;
} symbol_t;

typedef struct scope
{
    struct scope *parent;
    arena_t *arena;
    map_t *symbols;
} scope_t;

scope_t *
scope_new(arena_t *arena);

symbol_t *
symbol_new(arena_t *arena, string_view_t id);

symbol_t *
scope_lookup(scope_t *scope, string_view_t id);

void
scope_insert(scope_t *scope, symbol_t *symbol);

scope_t *
scope_push(scope_t *scope);

scope_t *
scope_pop(scope_t *scope);

#endif /* SCOPE_H */

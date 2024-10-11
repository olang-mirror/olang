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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scope.h"

scope_t *
scope_new(arena_t *arena)
{
    assert(arena);
    scope_t *scope = (scope_t *)arena_alloc(arena, sizeof(scope_t));
    if (scope == NULL) {
        fprintf(
            stderr, "[FATAL] Out of memory: scope_new: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    scope->arena = arena;
    scope->symbols = map_new(arena);

    // FIXME: create a list_new function to avoid spreading this.
    list_t *children = (list_t *)arena_alloc(arena, sizeof(list_t));

    if (children == NULL) {
        fprintf(
            stderr, "[FATAL] Out of memory: scope_new: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    list_init(children, arena);

    scope->children = children;
    return scope;
}

symbol_t *
symbol_new(arena_t *arena, string_view_t id, type_t *type)
{
    assert(arena);
    symbol_t *symbol = (symbol_t *)arena_alloc(arena, sizeof(symbol_t));
    if (symbol == NULL) {
        fprintf(
            stderr, "[FATAL] Out of memory: symbol_new: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    symbol->id = id;
    symbol->type = type;
    return symbol;
}

symbol_t *
scope_lookup(scope_t *scope, string_view_t id)
{
    assert(scope);
    while (scope != NULL) {

        char cstr_id[id.size + 1];
        cstr_id[id.size] = 0;
        memcpy(cstr_id, id.chars, id.size);

        symbol_t *symbol = (symbol_t *)map_get(scope->symbols, cstr_id);
        if (symbol != NULL) {
            return symbol;
        }
        scope = scope->parent;
    }
    return NULL;
}

void
scope_insert(scope_t *scope, symbol_t *symbol)
{
    assert(scope);
    assert(symbol);

    char id[symbol->id.size + 1];
    id[symbol->id.size] = 0;
    memcpy(id, symbol->id.chars, symbol->id.size);

    map_put(scope->symbols, id, symbol);
}

scope_t *
scope_push(scope_t *scope)
{
    assert(scope);

    scope_t *child = scope_new(scope->arena);
    child->parent = scope;

    list_append(scope->children, child);

    return child;
}

scope_t *
scope_pop(scope_t *scope)
{
    assert(scope);
    assert(scope->parent);
    return scope->parent;
}

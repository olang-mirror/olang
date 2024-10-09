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
#include "type.h"
#include "assert.h"

type_t *
type_new_unknown(arena_t *arena, string_view_t id)
{
    type_t *type = arena_alloc(arena, sizeof(type_t));
    assert(type);

    type->kind = TYPE_UNKNOWN;
    type->id = id;
    return type;
}

type_t *
type_new_ptr(arena_t *arena, string_view_t id, type_t *ref_type)
{
    type_t *type = arena_alloc(arena, sizeof(type_t));
    assert(type);

    type->kind = TYPE_PTR;
    type->id = id;
    type->as_ptr.type = ref_type;
    return type;
}

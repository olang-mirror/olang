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
#ifndef TYPE_H
#define TYPE_H
#include "arena.h"
#include "string_view.h"

typedef union type type_t;

typedef enum
{
    TYPE_UNKNOWN,
    TYPE_PRIMITIVE,
    TYPE_PTR
} type_kind_t;

typedef enum
{
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64
} type_primitive_kind_t;

typedef struct type_primitive
{
    type_kind_t _type_kind;
    string_view_t id;
    type_primitive_kind_t kind;
    short size;
} type_primitive_t;

typedef struct type_unknown
{
    type_kind_t _type_kind;
    string_view_t id;
} type_unknown_t;

typedef struct type_ptr
{
    type_kind_t _type_kind;
    string_view_t id;
    type_t *type;
} type_ptr_t;

typedef union type
{
    struct
    {
        type_kind_t kind;
        string_view_t id;
    };
    type_unknown_t as_unknown;
    type_primitive_t as_primitive;
    type_ptr_t as_ptr;
} type_t;

type_t *
type_new_unknown(arena_t *arena, string_view_t id);

type_t *
type_new_ptr(arena_t *arena, string_view_t id, type_t *type);
#endif

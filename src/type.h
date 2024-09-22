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
#include "string_view.h"
typedef enum
{
    TYPE_PRIMITIVE
} type_kind_t;

typedef enum
{
    TYPE_U32
} type_primitive_kind_t;

typedef struct type_primitive
{
    short size;
    type_primitive_kind_t kind;
} type_primitive_t;

typedef struct type
{
    string_view_t id;
    type_kind_t kind;
    union
    {
        type_primitive_t as_primitive;
    };
} type_t;

type_t
type_from_id(string_view_t id);
#endif

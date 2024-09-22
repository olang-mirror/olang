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

type_t
type_from_id(string_view_t id)
{
    type_t type = { 0 };
    if (string_view_eq_to_cstr(id, "u8")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 1;
        type.as_primitive.kind = TYPE_U8;
        return type;
    }
    if (string_view_eq_to_cstr(id, "u16")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 2;
        type.as_primitive.kind = TYPE_U16;
        return type;
    }
    if (string_view_eq_to_cstr(id, "u32")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 4;
        type.as_primitive.kind = TYPE_U32;
        return type;
    }
    if (string_view_eq_to_cstr(id, "u64")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 8;
        type.as_primitive.kind = TYPE_U64;
        return type;
    }

    // FIXME: handle user defined types
    assert(0 && "unknown type");
}

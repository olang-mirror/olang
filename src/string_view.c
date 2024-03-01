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
#include "string_view.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool
string_view_eq_to_cstr(string_view_t str, char *cstr)
{
    size_t cstr_len = strlen(cstr);
    if (str.size != cstr_len) {
        return false;
    }

    size_t i = 0;
    while (i < cstr_len && str.chars[i] == cstr[i]) {
        i++;
    }
    return i == cstr_len;
}

uint32_t
string_view_to_u32(string_view_t str)
{
    char ret[str.size + 1];
    ret[str.size + 1] = 0;
    memcpy(ret, str.chars, str.size);
    return atoi(ret);
}

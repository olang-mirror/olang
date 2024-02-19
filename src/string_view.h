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
#ifndef STRING_VIEW_T
#define STRING_VIEW_T

#include <stdbool.h>
#include <stddef.h>

typedef struct string_view
{
    char *chars;
    size_t size;

} string_view_t;

// TODO: missing unit test
bool
string_view_eq_to_cstr(string_view_t str, char *cstr);

#endif /* STRING_VIEW_T */

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
#define MUNIT_ENABLE_ASSERT_ALIASES
#include "munit.h"
#include "string_view.h"

#include <stdio.h>
#include <string.h>

static MunitResult
string_view_eq_to_cstr_test(const MunitParameter params[], void *user_data_or_fixture)
{
    char *name = "John Doe";

    string_view_t str = { .chars = name, .size = strlen(name) };

    assert_true(string_view_eq_to_cstr(str, "John Doe"));
    assert_false(string_view_eq_to_cstr(str, "Doe"));

    char *return_stmt = "return EXIT_SUCCESS;";

    str = (string_view_t){ .chars = return_stmt + 7, .size = 12 };
    assert_true(string_view_eq_to_cstr(str, "EXIT_SUCCESS"));

    return MUNIT_OK;
}

static MunitResult
string_view_to_u32_test(const MunitParameter params[], void *user_data_or_fixture)
{
    char *number = "69";

    string_view_t str = { .chars = number, .size = strlen(number) };

    assert_uint32(string_view_to_u32(str), ==, 69);

    str = (string_view_t){ .chars = "39;", .size = 2 };

    assert_uint32(string_view_to_u32(str), ==, 39);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    { "/eq_to_cstr_test", string_view_eq_to_cstr_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/to_u32_test", string_view_to_u32_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = { "/string_view", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

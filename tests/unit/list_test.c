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
#include "arena.h"
#include "list.h"
#include "munit.h"
#include <stdio.h>

static MunitResult
list_append_test(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(sizeof(list_item_t));

    list_t list;
    list_init(&list, &arena);

    munit_assert_int(list_size(&list), ==, 0);

    int value = 42;
    list_append(&list, &value);

    munit_assert_int(list_size(&list), ==, 1);

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitResult
list_get_test(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(sizeof(list_item_t) * 3);

    list_t list;
    list_init(&list, &arena);

    int a = 1;
    int b = 2;
    int c = 3;

    list_append(&list, &a);
    list_append(&list, &b);
    list_append(&list, &c);

    munit_assert_ptr_equal(list_get(&list, 0)->value, (void *)&a);
    munit_assert_ptr_equal(list_get(&list, 1)->value, (void *)&b);
    munit_assert_ptr_equal(list_get(&list, 2)->value, (void *)&c);

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitResult
list_next_test(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(sizeof(list_item_t) * 3);

    list_t list;
    list_init(&list, &arena);

    int a = 1;
    int b = 2;
    int c = 3;

    list_append(&list, &a);
    list_append(&list, &b);
    list_append(&list, &c);

    list_item_t *item_a = list_head(&list);
    list_item_t *item_b = list_next(item_a);
    list_item_t *item_c = list_next(item_b);

    munit_assert_ptr_equal(item_a->value, (void *)&a);
    munit_assert_ptr_equal(item_b->value, (void *)&b);
    munit_assert_ptr_equal(item_c->value, (void *)&c);
    munit_assert_ptr_null(list_next(item_c));

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitTest tests[] = { { "/list_append_test", list_append_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
                             { "/list_get_test", list_get_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
                             { "/list_next_test", list_next_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
                             { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL } };

static const MunitSuite suite = { "/cli_test", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

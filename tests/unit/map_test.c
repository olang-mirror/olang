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
#include "map.h"
#include "munit.h"
#include <stdio.h>

#define MAP_TEST_ARENA_CAPACITY (1024 * 16)

static MunitResult
test_create_new(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(MAP_TEST_ARENA_CAPACITY);

    map_t *map = map_new(&arena);

    assert_not_null(map);

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitResult
test_map_put_and_get(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(MAP_TEST_ARENA_CAPACITY);
    map_t *map = map_new(&arena);

    int n1 = 1;
    int n2 = 2;

    map_put(map, "n1", (void *)&n1);
    map_put(map, "n2", (void *)&n2);

    assert_int(map->size, ==, 2);
    assert_int(*((int *)map_get(map, "n1")), ==, n1);
    assert_int(*((int *)map_get(map, "n2")), ==, n2);

    map_put(map, "n1", (void *)&n2);

    assert_int(*((int *)map_get(map, "n1")), ==, n2);

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitResult
test_map_get_kvs(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(MAP_TEST_ARENA_CAPACITY);
    map_t *map = map_new(&arena);

    int n1 = 1;
    int n2 = 2;

    map_put(map, "n1", (void *)&n1);
    map_put(map, "n2", (void *)&n2);

    assert_int(map->size, ==, 2);

    map_kv_t map_kvs[map->size];

    map_get_kvs(map, (map_kv_t **)map_kvs);

    assert_string_equal(map_kvs[0].key, "n1");
    assert_int(*((int *)(map_kvs[0].value)), ==, 1);

    assert_string_equal(map_kvs[1].key, "n2");
    assert_int(*((int *)(map_kvs[1].value)), ==, 2);

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    { "/test_create_new", test_create_new, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/test_map_put_and_get", test_map_put_and_get, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = { "/map", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

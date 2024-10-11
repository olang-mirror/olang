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
#include "munit.h"

static MunitResult
arena_alloc_test(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(ARENA_ALIGNMENT_BYTES * 2);

    uint8_t *a = arena_alloc(&arena, sizeof(uint8_t));
    *a = 1;

    uint8_t *b = arena_alloc(&arena, sizeof(uint8_t));
    *b = 2;

    munit_assert_int(*a, ==, 1);
    munit_assert_int(*b, ==, 2);

    arena_release(&arena);

    uint8_t *c = arena_alloc(&arena, sizeof(uint8_t));
    *c = 3;

    munit_assert_int(*c, ==, 3);
    munit_assert_int(*a, ==, 3);
    munit_assert_int(*b, ==, 2);

    munit_assert_ptr_not_null(arena_alloc(&arena, sizeof(int)));
    munit_assert_ptr_null(arena_alloc(&arena, 1));

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitResult
arena_padding_test(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(512);

    // Allocated bytes is < ARENA_ALIGNMENT_BYTES
    uint8_t *a = arena_alloc(&arena, sizeof(uint8_t));
    uint8_t *b = arena_alloc(&arena, sizeof(uint8_t));

    munit_assert_int((b - a) % ARENA_ALIGNMENT_BYTES, ==, 0);
    munit_assert_int(b - a, ==, ARENA_ALIGNMENT_BYTES);

    arena_release(&arena);

    // Allocated bytes is == ARENA_ALIGNMENT_BYTES
    a = arena_alloc(&arena, ARENA_ALIGNMENT_BYTES);
    b = arena_alloc(&arena, sizeof(uint8_t));

    munit_assert_int((b - a) % ARENA_ALIGNMENT_BYTES, ==, 0);
    munit_assert_int(b - a, ==, ARENA_ALIGNMENT_BYTES);

    arena_release(&arena);

    // Allocated bytes is > ARENA_ALIGNMENT_BYTES
    a = arena_alloc(&arena, ARENA_ALIGNMENT_BYTES + 1);
    b = arena_alloc(&arena, sizeof(uint8_t));

    munit_assert_int((b - a) % ARENA_ALIGNMENT_BYTES, ==, 0);
    munit_assert_int(b - a, ==, ARENA_ALIGNMENT_BYTES * 2);

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    { "/arena_alloc_test",
      arena_alloc_test,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL },
    { "/arena_padding_test",
      arena_padding_test,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = { "/arena",
                                  tests,
                                  NULL,
                                  1,
                                  MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

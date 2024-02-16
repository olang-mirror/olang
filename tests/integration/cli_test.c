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
#include "cli_runner.h"
#include "munit.h"

static MunitResult
test_cli_hello_file(const MunitParameter params[], void *user_data_or_fixture)
{
    cli_result_t compilation_result = cli_runner_compile_file("../../examples/hello.olang");
    munit_assert_int(compilation_result.exit_code, ==, 0);
    return MUNIT_OK;
}

static MunitTest tests[] = { { "/test_cli_hello_file", test_cli_hello_file, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
                             { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL } };

static const MunitSuite suite = { "/cli_test", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

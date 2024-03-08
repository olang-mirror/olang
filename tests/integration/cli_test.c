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
test_cli_dump_tokens(const MunitParameter params[], void *user_data_or_fixture)
{
    cli_result_t compilation_result = cli_runner_compiler_dump_tokens("../../examples/main_exit.0");
    munit_assert_int(compilation_result.exit_code, ==, 0);
    munit_assert_string_equal(compilation_result.compiler_output,
                              "../../examples/main_exit.0:1:1: <fn>\n"
                              "../../examples/main_exit.0:1:4: <identifier>\n"
                              "../../examples/main_exit.0:1:8: <(>\n"
                              "../../examples/main_exit.0:1:9: <)>\n"
                              "../../examples/main_exit.0:1:10: <:>\n"
                              "../../examples/main_exit.0:1:12: <identifier>\n"
                              "../../examples/main_exit.0:1:16: <{>\n"
                              "../../examples/main_exit.0:1:17: <line_feed>\n"
                              "../../examples/main_exit.0:2:3: <return>\n"
                              "../../examples/main_exit.0:2:10: <number>\n"
                              "../../examples/main_exit.0:2:11: <line_feed>\n"
                              "../../examples/main_exit.0:3:1: <}>\n"
                              "../../examples/main_exit.0:3:2: <line_feed>\n"
                              "../../examples/main_exit.0:4:1: <EOF>\n");
    return MUNIT_OK;
}

static MunitTest tests[] = {
    { "/test_cli_dump_tokens", test_cli_dump_tokens, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = { "/cli_test", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

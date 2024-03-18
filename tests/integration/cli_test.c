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
#include <stdio.h>

static MunitResult
test_cli_dump_tokens_example_main_exit(const MunitParameter params[], void *user_data_or_fixture)
{
    cli_result_t compilation_result = cli_runner_compiler_dump_tokens("../../examples/main_exit.ol");
    munit_assert_int(compilation_result.exec.exit_code, ==, 0);
    munit_assert_string_equal(compilation_result.exec.stdout_buf,
                              "../../examples/main_exit.ol:1:1: <fn>\n"
                              "../../examples/main_exit.ol:1:4: <identifier>\n"
                              "../../examples/main_exit.ol:1:8: <(>\n"
                              "../../examples/main_exit.ol:1:9: <)>\n"
                              "../../examples/main_exit.ol:1:10: <:>\n"
                              "../../examples/main_exit.ol:1:12: <identifier>\n"
                              "../../examples/main_exit.ol:1:16: <{>\n"
                              "../../examples/main_exit.ol:1:17: <line_feed>\n"
                              "../../examples/main_exit.ol:2:3: <return>\n"
                              "../../examples/main_exit.ol:2:10: <number>\n"
                              "../../examples/main_exit.ol:2:11: <line_feed>\n"
                              "../../examples/main_exit.ol:3:1: <}>\n"
                              "../../examples/main_exit.ol:3:2: <line_feed>\n"
                              "../../examples/main_exit.ol:4:1: <EOF>\n");
    return MUNIT_OK;
}

static MunitResult
test_cli_dump_tokens_example_expression(const MunitParameter params[], void *user_data_or_fixture)
{
    cli_result_t compilation_result = cli_runner_compiler_dump_tokens("../../examples/expression.ol");
    munit_assert_int(compilation_result.exec.exit_code, ==, 0);
    munit_assert_string_equal(compilation_result.exec.stdout_buf,
                              "../../examples/expression.ol:1:1: <fn>\n"
                              "../../examples/expression.ol:1:4: <identifier>\n"
                              "../../examples/expression.ol:1:8: <(>\n"
                              "../../examples/expression.ol:1:9: <)>\n"
                              "../../examples/expression.ol:1:10: <:>\n"
                              "../../examples/expression.ol:1:12: <identifier>\n"
                              "../../examples/expression.ol:1:16: <{>\n"
                              "../../examples/expression.ol:1:17: <line_feed>\n"
                              "../../examples/expression.ol:2:3: <return>\n"
                              "../../examples/expression.ol:2:10: <(>\n"
                              "../../examples/expression.ol:2:11: <number>\n"
                              "../../examples/expression.ol:2:14: <+>\n"
                              "../../examples/expression.ol:2:16: <number>\n"
                              "../../examples/expression.ol:2:18: <*>\n"
                              "../../examples/expression.ol:2:20: <number>\n"
                              "../../examples/expression.ol:2:21: <)>\n"
                              "../../examples/expression.ol:2:23: <->\n"
                              "../../examples/expression.ol:2:25: <(>\n"
                              "../../examples/expression.ol:2:26: <number>\n"
                              "../../examples/expression.ol:2:29: <->\n"
                              "../../examples/expression.ol:2:31: <(>\n"
                              "../../examples/expression.ol:2:32: <number>\n"
                              "../../examples/expression.ol:2:34: <+>\n"
                              "../../examples/expression.ol:2:36: <number>\n"
                              "../../examples/expression.ol:2:37: <)>\n"
                              "../../examples/expression.ol:2:39: </>\n"
                              "../../examples/expression.ol:2:41: <number>\n"
                              "../../examples/expression.ol:2:42: <)>\n"
                              "../../examples/expression.ol:2:43: <line_feed>\n"
                              "../../examples/expression.ol:3:1: <}>\n"
                              "../../examples/expression.ol:3:2: <line_feed>\n"
                              "../../examples/expression.ol:4:1: <EOF>\n");
    return MUNIT_OK;
}

static MunitResult
test_cli_compile_minimal_program(const MunitParameter params[], void *user_data_or_fixture)
{
    cli_result_t compilation_result = cli_runner_compiler_compile("../../examples/main_exit.ol");
    munit_assert_int(compilation_result.exec.exit_code, ==, 0);

    char *command_args[] = { compilation_result.binary_path, NULL };

    proc_exec_command_t command = { .path = command_args[0], .args = command_args };

    proc_exec(&command);

    remove(command_args[0]);

    munit_assert_int(command.result.exit_code, ==, 0);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    { "/test_cli_dump_tokens_example_main_exit",
      test_cli_dump_tokens_example_main_exit,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL },
    { "/test_cli_dump_tokens_example_expression",
      test_cli_dump_tokens_example_expression,
      NULL,
      NULL,
      MUNIT_TEST_OPTION_NONE,
      NULL },
    { "/test_cli_compile_minimal_program", test_cli_compile_minimal_program, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = { "/cli_test", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

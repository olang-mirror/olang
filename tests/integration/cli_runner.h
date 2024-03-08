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
#ifndef CLI_RUNNER_H
#define CLI_RUNNER_H
#include "proc_exec.h"

typedef struct cli_result_t
{
    char binary_path[255];
    proc_exec_result_t exec;
} cli_result_t;

cli_result_t
cli_runner_compiler_dump_tokens(char *src);

cli_result_t
cli_runner_compiler_compile(char *src);
#endif

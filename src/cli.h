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
#ifndef CLI_H
#define CLI_H
#include "string_view.h"
#include <stdint.h>
#include <stdio.h>

typedef struct cli_args
{
    int argc;
    char **argv;
} cli_args_t;

typedef struct cli_opts
{
    uint32_t options;
    char *compiler_path;
    char *file_path;
    string_view_t output_bin;
} cli_opts_t;

typedef enum
{
    CLI_OPT_DUMP_TOKENS = 1 << 0,
    CLI_OPT_OUTPUT = 1 << 1,
    CLI_OPT_SAVE_TEMPS = 1 << 2
} cli_opt_t;

cli_opts_t
cli_parse_args(int argc, char **argv);

void
cli_print_usage(FILE *stream, char *compiler_path);

#endif /* CLI_H */

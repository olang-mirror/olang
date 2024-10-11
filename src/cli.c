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
#include "cli.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *
cli_args_shift(cli_args_t *args);

static void
cli_opts_parse_output(cli_opts_t *opts, cli_args_t *args);

static void
cli_opts_parse_arch(cli_opts_t *opts, cli_args_t *args);

static void
cli_opts_parse_sysroot(cli_opts_t *opts, cli_args_t *args);

cli_opts_t
cli_parse_args(int argc, char **argv)
{
    cli_args_t args = {
        .argc = argc,
        .argv = argv,
    };
    cli_opts_t opts = { 0 };

    opts.compiler_path = cli_args_shift(&args);

    char *arg = cli_args_shift(&args);
    while (arg != NULL) {
        if (strcmp(arg, "--dump-tokens") == 0) {
            opts.options |= CLI_OPT_DUMP_TOKENS;
        } else if (strcmp(arg, "--dump-ast") == 0) {
            opts.options |= CLI_OPT_DUMP_AST;
        } else if (strcmp(arg, "--save-temps") == 0) {
            opts.options |= CLI_OPT_SAVE_TEMPS;
        } else if (strcmp(arg, "-o") == 0) {
            cli_opts_parse_output(&opts, &args);
        } else if (strcmp(arg, "--arch") == 0) {
            opts.options |= CLI_OPT_ARCH;
            cli_opts_parse_arch(&opts, &args);
        } else if (strcmp(arg, "--sysroot") == 0) {
            opts.options |= CLI_OPT_SYSROOT;
            cli_opts_parse_sysroot(&opts, &args);
        } else {
            opts.filepath = arg;
        }
        arg = cli_args_shift(&args);
    }

    if (opts.options & CLI_OPT_OUTPUT || opts.options & CLI_OPT_DUMP_TOKENS || opts.options & CLI_OPT_DUMP_AST) {
        return opts;
    }

    cli_print_usage(stderr, opts.compiler_path);
    exit(EXIT_FAILURE);
    return opts;
}

static char *
cli_args_shift(cli_args_t *args)
{
    if (args->argc == 0)
        return NULL;
    --(args->argc);
    return *(args->argv)++;
}

static void
cli_opts_parse_output(cli_opts_t *opts, cli_args_t *args)
{
    assert(opts && "opts is required");
    assert(args && "args is required");

    char *output_bin = cli_args_shift(args);

    if (output_bin == NULL) {
        fprintf(stderr, "error: missing filename after '-o'\n");
        cli_print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    opts->options |= CLI_OPT_OUTPUT;
    opts->output_bin = string_view_from_cstr(output_bin);
}

static void
cli_opts_parse_arch(cli_opts_t *opts, cli_args_t *args)
{
    assert(opts && "opts is required");
    assert(args && "args is required");

    char *arch = cli_args_shift(args);

    if (arch == NULL) {
        fprintf(stderr, "error: missing architecture for arg '--arch': available options (x86_64 | aarch64)\n");
        cli_print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    opts->options |= CLI_OPT_ARCH;
    opts->arch = arch;
}

static void
cli_opts_parse_sysroot(cli_opts_t *opts, cli_args_t *args)
{
    assert(opts && "opts is required");
    assert(args && "args is required");

    char *sysroot = cli_args_shift(args);

    if (sysroot == NULL) {
        fprintf(stderr, "error: missing sysroot arg '--sysroot'\n");
        cli_print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    opts->options |= CLI_OPT_SYSROOT;
    opts->sysroot = sysroot;
}

void
cli_print_usage(FILE *stream, char *compiler_path)
{
    fprintf(stream,
            "Usage: %s [options] file...\n"
            "Options:\n"
            "  --dump-tokens    Display lexer token stream\n"
            "  --dump-ast       Display ast tree to stdout\n"
            "  --arch <arch>    Binary arch: default to x86_64 (x86_64 | aarch64)\n"
            "  --sysroot <dir>  System root dir where the GNU Assembler and GNU Linker are located: default to '/'\n"
            "  -o <file>        Compile program into a binary file\n"
            "  --save-temps     Keep temp files used to compile program\n",
            compiler_path);
}

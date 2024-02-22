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
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "string_view.h"

typedef struct cli_args
{
    int argc;
    char **argv;
} cli_args_t;

char *
cli_args_shift(cli_args_t *args);

typedef struct cli_opts
{
    bool dump_tokens;
    char *file_path;
} cli_opts_t;

void
print_usage(FILE *stream, char *prog);

static void
print_token(char *file_path, token_t *token);

string_view_t
read_entire_file(char *file_path);

int
main(int argc, char **argv)
{
    cli_args_t args = { .argc = argc, .argv = argv };
    cli_opts_t opts = { 0 };

    char *prog = cli_args_shift(&args);

    if (argc != 3) {
        print_usage(stderr, prog);
        return EXIT_FAILURE;
    }

    for (char *arg = cli_args_shift(&args); arg != NULL; arg = cli_args_shift(&args)) {
        if (strcmp(arg, "--dump-tokens") == 0) {
            opts.dump_tokens = true;
        } else {
            opts.file_path = arg;
        }
    }

    if (!opts.dump_tokens) {
        print_usage(stderr, prog);
        return EXIT_FAILURE;
    }

    string_view_t file_content = read_entire_file(opts.file_path);

    lexer_t lexer = { 0 };
    lexer_init(&lexer, file_content);

    token_t token = { 0 };
    lexer_next_token(&lexer, &token);
    while (token.kind != TOKEN_EOF) {
        print_token(opts.file_path, &token);
        lexer_next_token(&lexer, &token);
    }
    print_token(opts.file_path, &token);

    free(file_content.chars);

    return EXIT_SUCCESS;
}

char *
cli_args_shift(cli_args_t *args)
{
    if (args->argc == 0)
        return NULL;
    --(args->argc);
    return *(args->argv)++;
}

void
print_usage(FILE *stream, char *prog)
{
    fprintf(stream, "usage: %s <source.0> --dump-tokens\n", prog);
}

string_view_t
read_entire_file(char *file_path)
{
    FILE *stream = fopen(file_path, "rb");

    if (stream == NULL) {
        fprintf(stderr, "Could not open file %s: %s\n", file_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    string_view_t file_content = { 0 };

    fseek(stream, 0, SEEK_END);
    file_content.size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    file_content.chars = (char *)malloc(file_content.size);

    if (file_content.chars == NULL) {
        fprintf(stderr, "Could not read file %s: %s\n", file_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fread(file_content.chars, 1, file_content.size, stream);
    fclose(stream);

    return file_content;
}

static void
print_token(char *file_path, token_t *token)
{
    printf("%s:%lu:%lu: <%s>\n",
           file_path,
           token->location.row + 1,
           (token->location.offset - token->location.bol) + 1,
           token_kind_to_cstr(token->kind));
}

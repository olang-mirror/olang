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
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "codegen_linux_x86_64.h"
#include "lexer.h"
#include "parser.h"
#include "string_view.h"

// TODO: find a better solution to define the arena capacity
#define ARENA_CAPACITY (1024 * 1024)

typedef struct cli_args
{
    int argc;
    char **argv;
} cli_args_t;

char *
cli_args_shift(cli_args_t *args);

typedef enum
{
    CLI_OPT_DUMP_TOKENS = 1 << 0,
    CLI_OPT_OUTPUT = 1 << 1,
    CLI_OPT_SAVE_TEMPS = 1 << 2
} cli_opt_t;

typedef struct cli_opts
{
    uint32_t options;
    char *compiler_path;
    char *file_path;
    string_view_t output_bin;
} cli_opts_t;

void
print_usage(FILE *stream, char *compiler_path);

void
handle_dump_tokens(cli_opts_t *opts);

void
handle_codegen_linux_x86_64(cli_opts_t *opts);

static void
print_token(char *file_path, token_t *token);

string_view_t
read_entire_file(char *file_path, arena_t *arena);

void
cli_opts_parse_output(cli_opts_t *opts, cli_args_t *args);

int
main(int argc, char **argv)
{
    cli_args_t args = { .argc = argc, .argv = argv };
    cli_opts_t opts = { 0 };

    opts.compiler_path = cli_args_shift(&args);

    char *arg = cli_args_shift(&args);
    while (arg != NULL) {
        if (strcmp(arg, "--dump-tokens") == 0) {
            opts.options |= CLI_OPT_DUMP_TOKENS;
        } else if (strcmp(arg, "--save-temps") == 0) {
            opts.options |= CLI_OPT_SAVE_TEMPS;
        } else if (strcmp(arg, "-o") == 0) {
            cli_opts_parse_output(&opts, &args);
        } else {
            opts.file_path = arg;
        }
        arg = cli_args_shift(&args);
    }

    if (opts.options & CLI_OPT_OUTPUT) {
        handle_codegen_linux_x86_64(&opts);
        return EXIT_SUCCESS;
    }

    if (opts.options & CLI_OPT_DUMP_TOKENS) {
        handle_dump_tokens(&opts);
        return EXIT_SUCCESS;
    }

    print_usage(stderr, opts.compiler_path);
    return EXIT_FAILURE;
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
handle_dump_tokens(cli_opts_t *opts)
{
    if (opts->file_path == NULL) {
        print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    arena_t arena = arena_new(ARENA_CAPACITY);
    string_view_t file_content = read_entire_file(opts->file_path, &arena);

    lexer_t lexer = { 0 };
    lexer_init(&lexer, file_content);

    token_t token = { 0 };
    lexer_next_token(&lexer, &token);
    while (token.kind != TOKEN_EOF) {
        print_token(opts->file_path, &token);
        lexer_next_token(&lexer, &token);
    }
    print_token(opts->file_path, &token);

    arena_free(&arena);
}

void
handle_codegen_linux_x86_64(cli_opts_t *opts)
{
    if (opts->file_path == NULL) {
        print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    arena_t arena = arena_new(ARENA_CAPACITY);
    lexer_t lexer = { 0 };
    parser_t parser = { 0 };

    string_view_t file_content = read_entire_file(opts->file_path, &arena);
    lexer_init(&lexer, file_content);
    parser_init(&parser, &lexer, &arena, opts->file_path);

    ast_node_t *ast = parser_parse_fn_definition(&parser);

    char asm_file[opts->output_bin.size + 3];
    sprintf(asm_file, "" SV_FMT ".s", SV_ARG(opts->output_bin));

    FILE *out = fopen(asm_file, "w");
    assert(out);
    codegen_linux_x86_64_emit_program(out, ast);
    fclose(out);

    char command[512];
    sprintf(command, "as %s -o " SV_FMT ".o", asm_file, SV_ARG(opts->output_bin));
    system(command);

    sprintf(command, "ld " SV_FMT ".o -o " SV_FMT "", SV_ARG(opts->output_bin), SV_ARG(opts->output_bin));
    system(command);

    if (!(opts->options & CLI_OPT_SAVE_TEMPS)) {
        char output_file[256];

        sprintf(output_file, "" SV_FMT ".s", SV_ARG(opts->output_bin));
        remove(output_file);

        sprintf(output_file, "" SV_FMT ".o", SV_ARG(opts->output_bin));
        remove(output_file);
    }

    arena_free(&arena);
}

void
print_usage(FILE *stream, char *compiler_path)
{
    fprintf(stream,
            "Usage: %s [options] file...\n"
            "Options:\n"
            "  --dump-tokens\t\tDisplay lexer token stream\n"
            "  -o <file>\t\tCompile program into a binary file\n"
            "  --save-temps\t\tKeep temp files used to compile program\n",
            compiler_path);
}

string_view_t
read_entire_file(char *file_path, arena_t *arena)
{
    FILE *stream = fopen(file_path, "rb");

    if (stream == NULL) {
        fprintf(stderr, "error: could not open file %s: %s\n", file_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    string_view_t file_content = { 0 };

    fseek(stream, 0, SEEK_END);
    file_content.size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    assert(file_content.size * 2 < ARENA_CAPACITY);

    file_content.chars = (char *)arena_alloc(arena, (size_t)file_content.size);

    if (file_content.chars == NULL) {
        fprintf(stderr, "error: could not read file %s: %s\n", file_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fread(file_content.chars, 1, file_content.size, stream);
    fclose(stream);

    return file_content;
}

void
cli_opts_parse_output(cli_opts_t *opts, cli_args_t *args)
{
    assert(opts && "opts is required");
    assert(args && "args is required");

    char *output_bin = cli_args_shift(args);

    if (output_bin == NULL) {
        fprintf(stderr, "error: missing filename after '-o'\n");
        print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    opts->options |= CLI_OPT_OUTPUT;
    opts->output_bin = string_view_from_cstr(output_bin);
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

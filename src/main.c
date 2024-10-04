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
#include "checker.h"
#include "cli.h"
#include "codegen_linux_aarch64.h"
#include "codegen_linux_x86_64.h"
#include "lexer.h"
#include "parser.h"
#include "pretty_print_ast.h"
#include "string_view.h"

// TODO: find a better solution to define the arena capacity
#define ARENA_CAPACITY (1024 * 1024)

void
handle_dump_tokens(cli_opts_t *opts);

void
handle_dump_ast(cli_opts_t *opts);

void
handle_codegen_linux(cli_opts_t *opts);

static void
print_token(char *filepath, token_t *token);

source_code_t
read_entire_file(char *filepath, arena_t *arena);

int
main(int argc, char **argv)
{
    cli_opts_t opts = cli_parse_args(argc, argv);

    if (opts.options & CLI_OPT_DUMP_TOKENS) {
        handle_dump_tokens(&opts);
        return EXIT_SUCCESS;
    }

    if (opts.options & CLI_OPT_DUMP_AST) {
        handle_dump_ast(&opts);
        return EXIT_SUCCESS;
    }

    if (opts.options & CLI_OPT_OUTPUT) {
        handle_codegen_linux(&opts);
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

void
handle_dump_tokens(cli_opts_t *opts)
{
    if (opts->filepath == NULL) {
        cli_print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    arena_t arena = arena_new(ARENA_CAPACITY);
    source_code_t src = read_entire_file(opts->filepath, &arena);

    lexer_t lexer = { 0 };
    lexer_init(&lexer, src);

    token_t token = { 0 };
    lexer_next_token(&lexer, &token);
    while (token.kind != TOKEN_EOF) {
        print_token(opts->filepath, &token);
        lexer_next_token(&lexer, &token);
    }
    print_token(opts->filepath, &token);

    arena_free(&arena);
}

void
handle_dump_ast(cli_opts_t *opts)
{
    if (opts->filepath == NULL) {
        cli_print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    arena_t arena = arena_new(ARENA_CAPACITY);
    lexer_t lexer = { 0 };
    parser_t parser = { 0 };

    source_code_t src = read_entire_file(opts->filepath, &arena);

    lexer_init(&lexer, src);
    parser_init(&parser, &lexer, &arena, opts->filepath);

    ast_node_t *ast = parser_parse_translation_unit(&parser);

    pretty_print_ast(ast);
}

void
handle_codegen_linux(cli_opts_t *opts)
{
    if (opts->filepath == NULL) {
        cli_print_usage(stderr, opts->compiler_path);
        exit(EXIT_FAILURE);
    }

    arena_t arena = arena_new(ARENA_CAPACITY);
    lexer_t lexer = { 0 };
    parser_t parser = { 0 };

    source_code_t src = read_entire_file(opts->filepath, &arena);
    lexer_init(&lexer, src);
    parser_init(&parser, &lexer, &arena, opts->filepath);

    ast_node_t *ast = parser_parse_translation_unit(&parser);

    checker_t *checker = checker_new(&arena);
    checker_check(checker, ast);

    char asm_file[opts->output_bin.size + 3];
    sprintf(asm_file, "" SV_FMT ".s", SV_ARG(opts->output_bin));

    FILE *out = fopen(asm_file, "w");
    assert(out);

    if (!(opts->options & CLI_OPT_ARCH)) {
        codegen_x86_64_t codegen = { 0 };
        codegen_linux_x86_64_init(&codegen, &arena, out);
        codegen_linux_x86_64_emit_translation_unit(&codegen, ast);
    } else {
        if (strcmp(opts->arch, "x86_64") == 0) {
            codegen_x86_64_t codegen = { 0 };
            codegen_linux_x86_64_init(&codegen, &arena, out);
            codegen_linux_x86_64_emit_translation_unit(&codegen, ast);
        } else if (strcmp(opts->arch, "aarch64") == 0) {
            codegen_linux_aarch64_emit_translation_unit(out, ast);
        } else {
            fprintf(stderr, "error: architecture '%s' not supported\n", opts->arch);
            cli_print_usage(stderr, opts->compiler_path);
            exit(EXIT_FAILURE);
        }
    }

    fclose(out);

    if (!(opts->options & CLI_OPT_SYSROOT)) {
        opts->sysroot = "";
    }

    char command[512];
    sprintf(command, "%s/bin/as %s -o " SV_FMT ".o", opts->sysroot, asm_file, SV_ARG(opts->output_bin));

    int exit_code = system(command);

    if (exit_code != 0) {
        exit(exit_code);
    }

    sprintf(command,
            "%s/bin/ld " SV_FMT ".o -o " SV_FMT "",
            opts->sysroot,
            SV_ARG(opts->output_bin),
            SV_ARG(opts->output_bin));

    exit_code = system(command);

    if (exit_code != 0) {
        exit(exit_code);
    }

    if (!(opts->options & CLI_OPT_SAVE_TEMPS)) {
        char output_file[256];

        sprintf(output_file, "" SV_FMT ".s", SV_ARG(opts->output_bin));
        remove(output_file);

        sprintf(output_file, "" SV_FMT ".o", SV_ARG(opts->output_bin));
        remove(output_file);
    }

    arena_free(&arena);
}

source_code_t
read_entire_file(char *filepath, arena_t *arena)
{
    FILE *stream = fopen(filepath, "rb");

    if (stream == NULL) {
        fprintf(stderr, "error: could not open file %s: %s\n", filepath, strerror(errno));
        exit(EXIT_FAILURE);
    }

    string_view_t code = { 0 };

    fseek(stream, 0, SEEK_END);
    code.size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    assert(code.size * 2 < ARENA_CAPACITY);

    code.chars = (char *)arena_alloc(arena, (size_t)code.size);

    if (code.chars == NULL) {
        fprintf(stderr, "error: could not read file %s: %s\n", filepath, strerror(errno));
        exit(EXIT_FAILURE);
    }

    size_t read_bytes = fread(code.chars, 1, code.size, stream);

    if (read_bytes != code.size) {
        fprintf(stderr, "error: failed to read all file bytes %s\n", filepath);
        exit(EXIT_FAILURE);
    }

    fclose(stream);

    return (source_code_t){ .filepath = filepath, .code = code };
}

static void
print_token(char *filepath, token_t *token)
{
    printf("%s:%lu:%lu: <%s>\n",
           filepath,
           token->cur.row + 1,
           (token->cur.offset - token->cur.bol) + 1,
           token_kind_to_cstr(token->kind));
}

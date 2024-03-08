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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

static bool
skip_expected_token(parser_t *parser, token_kind_t expected_kind);

static bool
expected_token(parser_t *parser, token_t *token, token_kind_t kind);

static bool
parser_parse_type(parser_t *parser, type_t *type);

static ast_node_t *
parser_parse_block(parser_t *parser);

static void
skip_line_feeds(lexer_t *lexer);

void
parser_init(parser_t *parser, lexer_t *lexer, arena_t *arena, char *file_path)
{
    assert(parser && "parser is required");
    assert(lexer && "lexer is required");
    assert(file_path && "file_path is required");
    parser->lexer = lexer;
    parser->arena = arena;
    parser->file_path = file_path;
}

ast_node_t *
parser_parse_fn_definition(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_FN)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    token_t fn_name_token;

    if (!expected_token(parser, &fn_name_token, TOKEN_IDENTIFIER)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    if (!skip_expected_token(parser, TOKEN_OPAREN)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    if (!skip_expected_token(parser, TOKEN_CPAREN)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    if (!skip_expected_token(parser, TOKEN_COLON)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    type_t fn_return_type;
    if (!parser_parse_type(parser, &fn_return_type)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    ast_node_t *block = parser_parse_block(parser);
    if (block == NULL) {
        return NULL;
    }

    return ast_new_node_fn_def(parser->arena, fn_name_token.value, fn_return_type, block);
}

static bool
parser_parse_type(parser_t *parser, type_t *type)
{
    token_t token;

    if (!expected_token(parser, &token, TOKEN_IDENTIFIER)) {
        return false;
    }

    if (string_view_eq_to_cstr(token.value, "u32")) {
        *type = TYPE_U32;
        return true;
    }

    return false;
}

static ast_node_t *
parser_parse_block(parser_t *parser)
{
    token_t number_token;
    if (!skip_expected_token(parser, TOKEN_OCURLY)) {
        return false;
    }

    skip_line_feeds(parser->lexer);

    ast_node_t *node_block = ast_new_node_block(parser->arena);

    if (!skip_expected_token(parser, TOKEN_RETURN)) {
        return false;
    }

    ast_node_t *node_return_stmt = ast_new_node_return_stmt(parser->arena);
    assert(node_return_stmt);

    if (!expected_token(parser, &number_token, TOKEN_NUMBER)) {
        return false;
    }

    ast_node_t *literal_node = ast_new_node_literal_u32(parser->arena, string_view_to_u32(number_token.value));
    assert(literal_node);

    node_return_stmt->data.as_return_stmt.data = literal_node;

    list_append(node_block->data.as_block.nodes, node_return_stmt);

    if (!skip_expected_token(parser, TOKEN_LF)) {
        return false;
    }

    skip_line_feeds(parser->lexer);

    if (!skip_expected_token(parser, TOKEN_CCURLY)) {
        return false;
    }

    return node_block;
}

static bool
skip_expected_token(parser_t *parser, token_kind_t expected_kind)
{
    token_t token;
    return expected_token(parser, &token, expected_kind);
}

static bool
expected_token(parser_t *parser, token_t *token, token_kind_t expected_kind)
{
    lexer_next_token(parser->lexer, token);

    if (token->kind != expected_kind) {
        fprintf(stderr,
                "%s:%lu:%lu: error: got '" SV_FMT "' token but expect <%s>\n",
                parser->file_path,
                token->location.row + 1,
                (token->location.offset - token->location.bol) + 1,
                SV_ARG(token->value),
                token_kind_to_cstr(expected_kind));

        string_view_t line = lexer_get_token_line(parser->lexer, token);
        fprintf(stderr, "" SV_FMT "\n", SV_ARG(line));
        fprintf(stderr, "%*s\n", (int)(token->location.offset - token->location.bol + 1), "^");

        exit(EXIT_FAILURE);
    }
    return true;
}

static void
skip_line_feeds(lexer_t *lexer)
{
    token_t token;
    lexer_peek_next(lexer, &token);

    while (token.kind == TOKEN_LF) {
        lexer_next_token(lexer, &token);
        lexer_peek_next(lexer, &token);
    }
}

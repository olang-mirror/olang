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

#include "ast.h"
#include "lexer.h"
#include "parser.h"

static bool
skip_expected_token(parser_t *parser, token_kind_t expected_kind);

static bool
expected_next_token(parser_t *parser, token_t *token, token_kind_t kind);

static bool
expected_token(parser_t *parser, token_t *token, token_kind_t kind);

static bool
parser_parse_type(parser_t *parser, type_t *type);

static ast_node_t *
parser_parse_block(parser_t *parser);

static ast_node_t *
parser_parse_return_stmt(parser_t *parser);

static ast_node_t *
parser_parse_if_stmt(parser_t *parser);

ast_node_t *
parser_parse_fn_definition(parser_t *parser);

static ast_node_t *
parser_parse_expr(parser_t *parser);

static ast_node_t *
parser_parse_factor(parser_t *parser);

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
parser_parse_program(parser_t *parser)
{
    skip_line_feeds(parser->lexer);
    ast_node_t *fn = parser_parse_fn_definition(parser);
    if (fn == NULL) {
        return NULL;
    }

    return ast_new_program(parser->arena, fn);
}

static ast_binary_op_kind_t
token_kind_to_binary_op_kind(token_kind_t kind)
{
    switch (kind) {
        case TOKEN_PLUS:
            return AST_BINOP_ADDITION;
        case TOKEN_DASH:
            return AST_BINOP_SUBTRACTION;
        case TOKEN_SLASH:
            return AST_BINOP_DIVISION;
        case TOKEN_STAR:
            return AST_BINOP_MULTIPLICATION;
        case TOKEN_PERCENT:
            return AST_BINOP_REMINDER;
        case TOKEN_BITWISE_LSHIFT:
            return AST_BINOP_BITWISE_LSHIFT;
        case TOKEN_BITWISE_RSHIFT:
            return AST_BINOP_BITWISE_RSHIFT;
        case TOKEN_CIRCUMFLEX:
            return AST_BINOP_BITWISE_XOR;
        case TOKEN_AND:
            return AST_BINOP_BITWISE_AND;
        case TOKEN_PIPE:
            return AST_BINOP_BITWISE_OR;
        case TOKEN_LT:
            return AST_BINOP_CMP_LT;
        case TOKEN_GT:
            return AST_BINOP_CMP_GT;
        case TOKEN_CMP_LEQ:
            return AST_BINOP_CMP_LEQ;
        case TOKEN_CMP_GEQ:
            return AST_BINOP_CMP_GEQ;
        case TOKEN_CMP_EQ:
            return AST_BINOP_CMP_EQ;
        case TOKEN_CMP_NEQ:
            return AST_BINOP_CMP_NEQ;
        case TOKEN_LOGICAL_AND:
            return AST_BINOP_LOGICAL_AND;
        case TOKEN_LOGICAL_OR:
            return AST_BINOP_LOGICAL_OR;
        default: {
            fprintf(stderr, "error: token kind (%s) not compatible with binary op kind\n", token_kind_to_cstr(kind));
            assert(false);
        }
    }
}

typedef enum
{
    BINOP_MIN_PREC,
    BINOP_LOGICAL_OR_PREC,
    BINOP_LOGICAL_AND_PREC,
    BINOP_BITWISE_OR_PREC,
    BINOP_BITWISE_XOR_PREC,
    BINOP_BITWISE_AND_PREC,
    BINOP_CMP_EQUALITY_PREC,
    BINOP_CMP_RELATIONAL_PREC,
    BINOP_BITWISE_SHIFT_PREC,
    BINOP_ADDITIVE_PREC,
    BINOP_MULTIPLICATIVE_PREC,
} binary_op_precedence_t;

static binary_op_precedence_t
get_binary_op_precedence(token_kind_t kind)
{
    switch (kind) {
        case TOKEN_PLUS:
        case TOKEN_DASH:
            return BINOP_ADDITIVE_PREC;
        case TOKEN_SLASH:
        case TOKEN_STAR:
        case TOKEN_PERCENT:
            return BINOP_MULTIPLICATIVE_PREC;
        case TOKEN_BITWISE_LSHIFT:
        case TOKEN_BITWISE_RSHIFT:
            return BINOP_BITWISE_SHIFT_PREC;
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_CMP_LEQ:
        case TOKEN_CMP_GEQ:
            return BINOP_CMP_RELATIONAL_PREC;
        case TOKEN_CMP_EQ:
        case TOKEN_CMP_NEQ:
            return BINOP_CMP_EQUALITY_PREC;
        case TOKEN_AND:
            return BINOP_BITWISE_AND_PREC;
        case TOKEN_CIRCUMFLEX:
            return BINOP_BITWISE_XOR_PREC;
        case TOKEN_PIPE:
            return BINOP_BITWISE_OR_PREC;
        case TOKEN_LOGICAL_AND:
            return BINOP_LOGICAL_AND_PREC;
        case TOKEN_LOGICAL_OR:
            return BINOP_LOGICAL_OR_PREC;
        default:
            assert(false);
    }
}

static ast_node_t *
parser_parse_expr_1(parser_t *parser, ast_node_t *lhs, size_t prev_precedence)
{
    token_t lookahead_token;
    lexer_peek_next(parser->lexer, &lookahead_token);

    while (token_kind_is_binary_op(lookahead_token.kind) &&
           get_binary_op_precedence(lookahead_token.kind) >= prev_precedence) {
        token_t token_op;
        lexer_next_token(parser->lexer, &token_op);

        ast_node_t *rhs = parser_parse_factor(parser);
        if (rhs == NULL) {
            return NULL;
        }

        lexer_peek_next(parser->lexer, &lookahead_token);

        while (token_kind_is_binary_op(lookahead_token.kind) &&
               get_binary_op_precedence(lookahead_token.kind) > get_binary_op_precedence(token_op.kind)) {
            rhs = parser_parse_expr_1(parser, rhs, get_binary_op_precedence(token_op.kind));
            lexer_peek_next(parser->lexer, &lookahead_token);
        }

        lhs = ast_new_node_bin_op(parser->arena, token_kind_to_binary_op_kind(token_op.kind), lhs, rhs);
        if (lhs == NULL) {
            return NULL;
        }
    }

    return lhs;
}

static ast_node_t *
parser_parse_expr(parser_t *parser)
{
    ast_node_t *lhs = parser_parse_factor(parser);
    if (lhs == NULL) {
        return NULL;
    }

    return parser_parse_expr_1(parser, lhs, BINOP_MIN_PREC);
}

static ast_node_t *
parser_parse_factor(parser_t *parser)
{
    token_t token;
    lexer_next_token(parser->lexer, &token);

    switch (token.kind) {
        case TOKEN_NUMBER:
            return ast_new_node_literal_u32(parser->arena, string_view_to_u32(token.value));

        case TOKEN_OPAREN: {
            ast_node_t *expr = parser_parse_expr(parser);
            if (expr == NULL) {
                return NULL;
            }

            if (!skip_expected_token(parser, TOKEN_CPAREN)) {
                return NULL;
            }

            return expr;
        }
        default: {
            fprintf(stderr, "error: parse_factor: unsupported or invalid token (%s)\n", token_kind_to_cstr(token.kind));
            assert(false);
        }
    }
}

ast_node_t *
parser_parse_fn_definition(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_FN)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    token_t fn_name_token;

    if (!expected_next_token(parser, &fn_name_token, TOKEN_IDENTIFIER)) {
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

    if (!expected_next_token(parser, &token, TOKEN_IDENTIFIER)) {
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
    if (!skip_expected_token(parser, TOKEN_OCURLY)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    ast_node_t *node_block = ast_new_node_block(parser->arena);
    if (node_block == NULL) {
        return NULL;
    }

    token_t next_token;

StartLoop:
    lexer_peek_next(parser->lexer, &next_token);
    ast_node_t *node = NULL;

    switch (next_token.kind) {
        case TOKEN_RETURN: {
            node = parser_parse_return_stmt(parser);
            break;
        }
        case TOKEN_IF: {
            node = parser_parse_if_stmt(parser);
            break;
        }
        default: {
            goto EndLoop;
        }
    }

    if (node == NULL) {
        return NULL;
    }

    list_append(node_block->as_block.nodes, node);

    goto StartLoop;
EndLoop:

    skip_line_feeds(parser->lexer);
    if (!skip_expected_token(parser, TOKEN_CCURLY)) {
        return NULL;
    }

    return node_block;
}

static ast_node_t *
parser_parse_return_stmt(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_RETURN)) {
        return NULL;
    }

    ast_node_t *node_return_stmt = ast_new_node_return_stmt(parser->arena);
    assert(node_return_stmt);

    ast_node_t *expr = parser_parse_expr(parser);
    if (expr == NULL) {
        return NULL;
    }

    node_return_stmt->as_return_stmt.data = expr;

    if (!skip_expected_token(parser, TOKEN_LF)) {
        return NULL;
    }
    skip_line_feeds(parser->lexer);

    return node_return_stmt;
}

static ast_node_t *
parser_parse_if_stmt(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_IF)) {
        return NULL;
    }

    ast_node_t *cond = parser_parse_expr(parser);

    if (cond == NULL) {
        return NULL;
    }

    ast_node_t *then = parser_parse_block(parser);

    if (then == NULL) {
        return NULL;
    }

    ast_node_t *_else = NULL;

    token_t next_token;
    lexer_next_token(parser->lexer, &next_token);

    // FIXME: We are not allowing line feed right after if block statement when
    //        the else branch is present.  We also noticed that if has the same
    //        problem which will be addressed later.

    if (next_token.kind == TOKEN_ELSE) {
        _else = parser_parse_block(parser);

        if (_else == NULL) {
            return NULL;
        }

    } else if (!expected_token(parser, &next_token, TOKEN_LF)) {
        return NULL;
    }

    ast_node_t *node_if_stmt = ast_new_node_if_stmt(parser->arena, cond, then, _else);

    assert(node_if_stmt);

    skip_line_feeds(parser->lexer);

    return node_if_stmt;
}

static bool
skip_expected_token(parser_t *parser, token_kind_t expected_kind)
{
    token_t token;
    return expected_next_token(parser, &token, expected_kind);
}

static bool
expected_next_token(parser_t *parser, token_t *token, token_kind_t expected_kind)
{
    lexer_next_token(parser->lexer, token);
    return expected_token(parser, token, expected_kind);
}

static bool
expected_token(parser_t *parser, token_t *token, token_kind_t expected_kind)
{
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

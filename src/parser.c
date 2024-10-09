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
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

static bool
skip_expected_token(parser_t *parser, token_kind_t expected_kind);

static bool
expected_next_token(parser_t *parser, token_t *token, token_kind_t kind);

static bool
expected_token(token_t *token, token_kind_t kind);

static type_t *
parser_parse_type(parser_t *parser);

static ast_node_t *
parser_parse_block(parser_t *parser);

static ast_node_t *
parser_parse_return_stmt(parser_t *parser);

static ast_node_t *
parser_parse_if_stmt(parser_t *parser);

static ast_node_t *
parser_parse_while_stmt(parser_t *parser);

static ast_node_t *
parser_parse_var_def(parser_t *parser);

static ast_node_t *
parser_parse_var_assign_stmt(parser_t *parser);

static ast_node_t *
parser_parse_fn_definition(parser_t *parser);

static list_t *
parser_parse_fn_args(parser_t *parser);

static list_t *
parser_parse_fn_params(parser_t *parser);

static ast_node_t *
parser_parse_expr(parser_t *parser);

static ast_node_t *
parser_parse_factor(parser_t *parser);

static void
skip_line_feeds(lexer_t *lexer);

static void
peek_next_non_lf_token(lexer_t *lexer, token_t *token);

void
parser_init(parser_t *parser, lexer_t *lexer, arena_t *arena)
{
    assert(parser && "parser is required");
    assert(lexer && "lexer is required");
    parser->lexer = lexer;
    parser->arena = arena;
}

ast_node_t *
parser_parse_translation_unit(parser_t *parser)
{
    token_t token;
    ast_node_t *translation_unit_node = ast_new_translation_unit(parser->arena);

    skip_line_feeds(parser->lexer);
    lexer_peek_next(parser->lexer, &token);

    while (token.kind != TOKEN_EOF) {
        ast_node_t *fn = parser_parse_fn_definition(parser);

        if (fn == NULL) {
            return NULL;
        }

        list_append(translation_unit_node->as_translation_unit.decls, fn);

        skip_line_feeds(parser->lexer);
        lexer_peek_next(parser->lexer, &token);
    }

    return translation_unit_node;
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

static ast_unary_op_kind_t
token_kind_to_unary_op_kind(token_kind_t token_kind)
{
    switch (token_kind) {
        case TOKEN_AND:
            return AST_UNARY_ADDRESSOF;
        case TOKEN_STAR:
            return AST_UNARY_DEREFERENCE;
        case TOKEN_PLUS:
            return AST_UNARY_POSITIVE;
        case TOKEN_DASH:
            return AST_UNARY_NEGATIVE;
        case TOKEN_TILDE:
            return AST_UNARY_BITWISE_NOT;
        case TOKEN_BANG:
            return AST_UNARY_LOGICAL_NOT;
        default:
            assert(false && "unable to covert the token_kind_t to unary_op_kind_t");
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

        lhs = ast_new_node_bin_op(parser->arena, token_op.loc, token_kind_to_binary_op_kind(token_op.kind), lhs, rhs);
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
            return ast_new_node_literal_u32(parser->arena, token.loc, string_view_to_u32(token.value));

        case TOKEN_ID: {
            token_t token_id = token;

            lexer_peek_next(parser->lexer, &token);

            if (token.kind == TOKEN_OPAREN) {
                list_t *args = parser_parse_fn_args(parser);
                return ast_new_node_fn_call(parser->arena, token_id.loc, token_id.value, args);
            }

            return ast_new_node_ref(parser->arena, token_id.loc, token_id.value);
        }
        case TOKEN_AND:
        case TOKEN_STAR:
        case TOKEN_PLUS:
        case TOKEN_DASH:
        case TOKEN_TILDE:
        case TOKEN_BANG: {
            ast_node_t *expr = parser_parse_expr(parser);
            if (expr == NULL) {
                return NULL;
            }

            ast_unary_op_kind_t kind = token_kind_to_unary_op_kind(token.kind);
            return ast_new_node_unary_op(parser->arena, token.loc, kind, expr);
        }

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

static list_t *
parser_parse_fn_args(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_OPAREN)) {
        return NULL;
    }

    list_t *args = arena_alloc(parser->arena, sizeof(list_t));
    if (args == NULL) {
        fprintf(stderr, "[FATAL] Out of memory: parser_parse_fn_args: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    list_init(args, parser->arena);

    skip_line_feeds(parser->lexer);

    token_t token;
    lexer_peek_next(parser->lexer, &token);

    bool is_not_first_arg = false;

    while (token.kind != TOKEN_CPAREN && token.kind != TOKEN_EOF) {
        if (is_not_first_arg && expected_token(&token, TOKEN_COMMA)) {
            lexer_next_token(parser->lexer, &token);
        }

        ast_node_t *expr = parser_parse_expr(parser);
        list_append(args, expr);

        skip_line_feeds(parser->lexer);
        lexer_peek_next(parser->lexer, &token);
        is_not_first_arg = true;
    }

    if (!skip_expected_token(parser, TOKEN_CPAREN)) {
        return NULL;
    }

    return args;
}

static list_t *
parser_parse_fn_params(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_OPAREN)) {
        return NULL;
    }

    list_t *params = arena_alloc(parser->arena, sizeof(list_t));
    if (params == NULL) {
        fprintf(stderr, "[FATAL] Out of memory: parser_parse_fn_params: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    list_init(params, parser->arena);

    skip_line_feeds(parser->lexer);

    token_t token;
    lexer_next_token(parser->lexer, &token);

    bool is_not_first_param = false;

    while (token.kind != TOKEN_CPAREN && token.kind != TOKEN_EOF) {
        if (is_not_first_param && expected_token(&token, TOKEN_COMMA)) {
            lexer_next_token(parser->lexer, &token);
        }

        if (!expected_token(&token, TOKEN_ID)) {
            return NULL;
        }

        type_t *type = parser_parse_type(parser);

        if (type == NULL) {
            return NULL;
        }

        ast_fn_param_t *param = ast_new_fn_param(parser->arena, token.value, type);
        list_append(params, param);

        skip_line_feeds(parser->lexer);
        lexer_next_token(parser->lexer, &token);
        is_not_first_param = true;
    }

    if (!expected_token(&token, TOKEN_CPAREN)) {
        return NULL;
    }

    return params;
}

ast_node_t *
parser_parse_fn_definition(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_FN)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    token_t fn_name_token;

    if (!expected_next_token(parser, &fn_name_token, TOKEN_ID)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    list_t *params = parser_parse_fn_params(parser);
    if (params == NULL) {
        return NULL;
    }

    type_t *ret_type = parser_parse_type(parser);

    if (ret_type == NULL) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    ast_node_t *block = parser_parse_block(parser);
    if (block == NULL) {
        return NULL;
    }

    return ast_new_node_fn_def(parser->arena, fn_name_token.loc, fn_name_token.value, params, ret_type, block);
}

static type_t *
parser_parse_type(parser_t *parser)
{
    skip_line_feeds(parser->lexer);

    if (!skip_expected_token(parser, TOKEN_COLON)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    token_t token;

    if (!expected_next_token(parser, &token, TOKEN_ID)) {
        return NULL;
    }

    token_t ptr_token;

    lexer_peek_next(parser->lexer, &ptr_token);

    type_t *type = type_new_unknown(parser->arena, token.value);

    if (ptr_token.kind == TOKEN_STAR) {
        if (!skip_expected_token(parser, TOKEN_STAR)) {
            return NULL;
        }
        string_view_t ptr_id = token.value;

        ptr_id.size = ptr_token.value.chars - token.value.chars + ptr_token.value.size;

        return type_new_ptr(parser->arena, ptr_id, type);
    }

    return type;
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
        case TOKEN_WHILE: {
            node = parser_parse_while_stmt(parser);
            break;
        }
        case TOKEN_VAR: {
            node = parser_parse_var_def(parser);
            break;
        }
        case TOKEN_ID: {
            lexer_lookahead(parser->lexer, &next_token, 2);
            if (!expected_token(&next_token, TOKEN_EQ)) {
                return NULL;
            }
            node = parser_parse_var_assign_stmt(parser);
            break;
        }
        case TOKEN_CCURLY: {
            goto EndLoop;
        }
        default: {
            // FIXME: write a better error message
            goto EndLoop;
        }
    }

    if (node == NULL) {
        return NULL;
    }

    if (!skip_expected_token(parser, TOKEN_LF)) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    list_append(node_block->as_block.nodes, node);

    goto StartLoop;
EndLoop:

    if (!skip_expected_token(parser, TOKEN_CCURLY)) {
        return NULL;
    }

    return node_block;
}

static ast_node_t *
parser_parse_return_stmt(parser_t *parser)
{
    token_t token_ret;

    if (!expected_next_token(parser, &token_ret, TOKEN_RETURN)) {
        return NULL;
    }

    ast_node_t *expr = parser_parse_expr(parser);
    if (expr == NULL) {
        return NULL;
    }

    ast_node_t *node_return_stmt = ast_new_node_return_stmt(parser->arena, token_ret.loc, expr);
    assert(node_return_stmt);

    return node_return_stmt;
}

static ast_node_t *
parser_parse_if_stmt(parser_t *parser)
{
    token_t token_if;
    if (!expected_next_token(parser, &token_if, TOKEN_IF)) {
        return NULL;
    }

    ast_node_t *cond = parser_parse_expr(parser);

    if (cond == NULL) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    ast_node_t *then = parser_parse_block(parser);

    if (then == NULL) {
        return NULL;
    }

    ast_node_t *_else = NULL;

    token_t next_token;
    peek_next_non_lf_token(parser->lexer, &next_token);

    if (next_token.kind == TOKEN_ELSE) {
        skip_line_feeds(parser->lexer);
        lexer_next_token(parser->lexer, &next_token);
        skip_line_feeds(parser->lexer);

        lexer_peek_next(parser->lexer, &next_token);

        if (next_token.kind == TOKEN_IF) {
            _else = parser_parse_if_stmt(parser);
        } else {
            _else = parser_parse_block(parser);
        }

        if (_else == NULL) {
            return NULL;
        }
    }

    ast_node_t *node_if_stmt = ast_new_node_if_stmt(parser->arena, token_if.loc, cond, then, _else);

    assert(node_if_stmt);

    return node_if_stmt;
}

static ast_node_t *
parser_parse_while_stmt(parser_t *parser)
{
    token_t token_while;
    if (!expected_next_token(parser, &token_while, TOKEN_WHILE)) {
        return NULL;
    }

    ast_node_t *cond = parser_parse_expr(parser);

    if (cond == NULL) {
        return NULL;
    }

    skip_line_feeds(parser->lexer);

    ast_node_t *then = parser_parse_block(parser);

    if (then == NULL) {
        return NULL;
    }

    token_t next_token;
    peek_next_non_lf_token(parser->lexer, &next_token);

    ast_node_t *node_while_stmt = ast_new_node_while_stmt(parser->arena, token_while.loc, cond, then);

    assert(node_while_stmt);

    return node_while_stmt;
}

static ast_node_t *
parser_parse_var_def(parser_t *parser)
{
    if (!skip_expected_token(parser, TOKEN_VAR)) {
        return NULL;
    }

    token_t token_id;
    if (!expected_next_token(parser, &token_id, TOKEN_ID)) {
        return NULL;
    }

    type_t *type = parser_parse_type(parser);
    if (type == NULL) {
        return NULL;
    }

    // FIXME: assignment must be optional according to the spec
    if (!skip_expected_token(parser, TOKEN_EQ)) {
        return NULL;
    }

    ast_node_t *expr = parser_parse_expr(parser);
    if (expr == NULL) {
        return NULL;
    }

    ast_node_t *var_node = ast_new_node_var_def(parser->arena, token_id.loc, token_id.value, type, expr);

    return var_node;
}

static ast_node_t *
parser_parse_var_assign_stmt(parser_t *parser)
{
    token_t token_id;

    if (!expected_next_token(parser, &token_id, TOKEN_ID)) {
        return NULL;
    }

    token_t token_eq;

    if (!expected_next_token(parser, &token_eq, TOKEN_EQ)) {
        return NULL;
    }

    ast_node_t *ref = ast_new_node_ref(parser->arena, token_id.loc, token_id.value);
    ast_node_t *expr = parser_parse_expr(parser);

    return ast_new_node_var_assign_stmt(parser->arena, token_eq.loc, ref, expr);
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
    return expected_token(token, expected_kind);
}

static bool
expected_token(token_t *token, token_kind_t expected_kind)
{
    if (token->kind != expected_kind) {
        fprintf(stderr,
                "%s:%lu:%lu: syntax error: got '" SV_FMT "' token but expect '%s'\n",
                token->loc.src.filepath,
                token_loc_to_lineno(token->loc),
                token_loc_to_colno(token->loc),
                SV_ARG(token->value),
                token_kind_to_cstr(expected_kind));

        fprintf(stderr, SV_FMT "\n", SV_ARG(token_loc_to_line(token->loc)));
        fprintf(stderr, "%*s\n", (int)token_loc_to_colno(token->loc), "^");

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

static void
peek_next_non_lf_token(lexer_t *lexer, token_t *token)
{
    lexer_cursor_t cur = lexer->cur;

    skip_line_feeds(lexer);
    lexer_peek_next(lexer, token);

    lexer->cur = cur;
}

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
#ifndef AST_H
#define AST_H

#include <stdint.h>

#include "arena.h"
#include "lexer.h"
#include "list.h"
#include "scope.h"
#include "string_view.h"
#include "type.h"

typedef union ast_node ast_node_t;

typedef enum
{
    AST_NODE_TRANSLATION_UNIT,
    AST_NODE_BLOCK,
    AST_NODE_FN_DEF,
    AST_NODE_FN_CALL,
    AST_NODE_VAR_DEF,
    AST_NODE_BINARY_OP,
    AST_NODE_VAR_ASSIGN_STMT,
    AST_NODE_RETURN_STMT,
    AST_NODE_IF_STMT,
    AST_NODE_WHILE_STMT,
    AST_NODE_LITERAL,
    AST_NODE_REF,
    AST_NODE_UNKNOWN
} ast_node_kind_t;

typedef struct ast_node_meta
{
    ast_node_kind_t kind;
    token_loc_t loc;
} ast_node_meta_t;

typedef struct ast_block
{
    ast_node_meta_t meta;
    list_t *nodes;
} ast_block_t;

typedef struct ast_translation_unit
{
    ast_node_meta_t meta;
    list_t *decls;
} ast_translation_unit_t;

typedef struct ast_fn_param
{
    string_view_t id;
    string_view_t type_id;
} ast_fn_param_t;

typedef struct ast_fn_definition
{
    ast_node_meta_t meta;
    string_view_t id;
    list_t *params;
    string_view_t return_type;
    ast_node_t *block;
    scope_t *scope;
} ast_fn_definition_t;

typedef struct ast_fn_call
{
    ast_node_meta_t meta;
    string_view_t id;
    list_t *args;
    scope_t *scope;
} ast_fn_call_t;

typedef struct ast_var_definition
{
    ast_node_meta_t meta;
    string_view_t id;
    string_view_t type;
    ast_node_t *value;
    scope_t *scope;
} ast_var_definition_t;

typedef enum
{
    AST_LITERAL_U32
} ast_literal_kind_t;

typedef struct ast_literal
{
    ast_node_meta_t meta;
    ast_literal_kind_t kind;
    union
    {
        uint32_t as_u32;
    };
} ast_literal_t;

typedef struct ast_ref
{
    ast_node_meta_t meta;
    string_view_t id;
    scope_t *scope;
} ast_ref_t;

typedef enum ast_binary_op_kind
{
    AST_BINOP_ADDITION,
    AST_BINOP_SUBTRACTION,
    AST_BINOP_MULTIPLICATION,
    AST_BINOP_DIVISION,
    AST_BINOP_REMINDER,
    AST_BINOP_BITWISE_LSHIFT,
    AST_BINOP_BITWISE_RSHIFT,
    AST_BINOP_BITWISE_XOR,
    AST_BINOP_BITWISE_AND,
    AST_BINOP_BITWISE_OR,
    AST_BINOP_CMP_LT,
    AST_BINOP_CMP_GT,
    AST_BINOP_CMP_LEQ,
    AST_BINOP_CMP_GEQ,
    AST_BINOP_CMP_EQ,
    AST_BINOP_CMP_NEQ,
    AST_BINOP_LOGICAL_AND,
    AST_BINOP_LOGICAL_OR,
} ast_binary_op_kind_t;

typedef struct ast_binary_op
{
    ast_node_meta_t meta;
    ast_binary_op_kind_t kind;
    ast_node_t *lhs;
    ast_node_t *rhs;
} ast_binary_op_t;

typedef struct ast_var_assign_stmt
{
    ast_node_meta_t meta;
    ast_node_t *ref;
    ast_node_t *expr;
} ast_var_assign_stmt_t;

typedef struct ast_return_stmt
{
    ast_node_meta_t meta;
    ast_node_t *expr;
} ast_return_stmt_t;

typedef struct ast_if_stmt
{
    ast_node_meta_t meta;
    ast_node_t *cond;
    ast_node_t *then;
    ast_node_t *_else;
} ast_if_stmt_t;

typedef struct ast_while_stmt
{
    ast_node_meta_t meta;
    ast_node_t *cond;
    ast_node_t *then;
} ast_while_stmt_t;

typedef union ast_node
{
    // inlined ast_node_meta_t struct.
    struct
    {
        ast_node_kind_t kind;
        token_loc_t loc;
    };
    ast_translation_unit_t as_translation_unit;
    ast_fn_definition_t as_fn_def;
    ast_fn_call_t as_fn_call;
    ast_var_definition_t as_var_def;
    ast_binary_op_t as_bin_op;
    ast_literal_t as_literal;
    ast_ref_t as_ref;
    ast_block_t as_block;
    ast_var_assign_stmt_t as_var_assign_stmt;
    ast_return_stmt_t as_return_stmt;
    ast_if_stmt_t as_if_stmt;
    ast_while_stmt_t as_while_stmt;
} ast_node_t;

ast_node_t *
ast_new_translation_unit(arena_t *arena);

ast_node_t *
ast_new_node_fn_def(arena_t *arena,
                    token_loc_t loc,
                    string_view_t id,
                    list_t *params,
                    string_view_t return_type,
                    ast_node_t *block);

ast_node_t *
ast_new_node_fn_call(arena_t *arena, token_loc_t loc, string_view_t id, list_t *args);

ast_node_t *
ast_new_node_var_def(arena_t *arena, token_loc_t loc, string_view_t id, string_view_t type, ast_node_t *value);

ast_node_t *
ast_new_node_bin_op(arena_t *arena, token_loc_t loc, ast_binary_op_kind_t kind, ast_node_t *lhs, ast_node_t *rhs);

ast_node_t *
ast_new_node_literal_u32(arena_t *arena, token_loc_t loc, uint32_t value);

ast_node_t *
ast_new_node_ref(arena_t *arena, token_loc_t loc, string_view_t id);

ast_node_t *
ast_new_node_var_assign_stmt(arena_t *arena, token_loc_t loc, ast_node_t *ref, ast_node_t *expr);

ast_node_t *
ast_new_node_return_stmt(arena_t *arena, token_loc_t loc, ast_node_t *expr);

ast_node_t *
ast_new_node_if_stmt(arena_t *arena, token_loc_t loc, ast_node_t *cond, ast_node_t *then, ast_node_t *_else);

ast_node_t *
ast_new_node_while_stmt(arena_t *arena, token_loc_t loc, ast_node_t *cond, ast_node_t *then);

ast_node_t *
ast_new_node_block(arena_t *arena);

ast_fn_param_t *
ast_new_fn_param(arena_t *arena, string_view_t id, string_view_t type_id);

#endif /* AST_H */

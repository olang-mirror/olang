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
#include "list.h"
#include "string_view.h"

typedef struct ast_node ast_node_t;

typedef enum
{
    AST_NODE_PROGRAM,
    AST_NODE_BLOCK,
    AST_NODE_FN_DEF,
    AST_NODE_RETURN_STMT,
    AST_NODE_LITERAL,
    AST_NODE_UNKNOWN
} ast_node_kind_t;

typedef enum
{
    TYPE_U32
} type_t;

typedef struct ast_block
{
    list_t *nodes;
} ast_block_t;

typedef struct ast_program
{
    ast_node_t *fn;
} ast_program_t;

typedef struct ast_fn_definition
{
    string_view_t identifier;
    type_t return_type;
    ast_node_t *block;
} ast_fn_definition_t;

typedef enum
{
    AST_LITERAL_U32
} ast_literal_kind_t;

typedef union
{
    uint32_t as_u32;
} ast_literal_value_t;

typedef struct ast_literal
{
    ast_literal_kind_t kind;
    ast_literal_value_t value;
} ast_literal_t;

typedef struct ast_return_stmt
{
    ast_node_t *data;
} ast_return_stmt_t;

typedef union
{
    ast_program_t as_program;
    ast_fn_definition_t as_fn_def;
    ast_literal_t as_literal;
    ast_block_t as_block;
    ast_return_stmt_t as_return_stmt;
} ast_node_data_t;

typedef struct ast_node
{
    ast_node_kind_t kind;
    ast_node_data_t data;
} ast_node_t;

ast_node_t *
ast_new_program(arena_t *arena, ast_node_t *fn_def);

ast_node_t *
ast_new_node_fn_def(arena_t *arena, string_view_t identifier, type_t return_type, ast_node_t *block);

ast_node_t *
ast_new_node_literal_u32(arena_t *arena, uint32_t value);

ast_node_t *
ast_new_node_return_stmt(arena_t *arena);

ast_node_t *
ast_new_node_block(arena_t *arena);

#endif /* AST_H */

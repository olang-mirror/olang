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
#include <stdint.h>

#include "arena.h"
#include "ast.h"
#include "string_view.h"

ast_node_t *
ast_new_node_fn_def(arena_t *arena, string_view_t identifier, type_t return_type, ast_node_t *block)
{
    ast_node_t *node_fn_def = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_fn_def);

    node_fn_def->kind = AST_NODE_FN_DEF;
    ast_fn_definition_t *fn_def = &node_fn_def->data.as_fn_def;

    fn_def->identifier = identifier;
    fn_def->return_type = return_type;
    fn_def->block = block;

    return node_fn_def;
}

ast_node_t *
ast_new_node_literal_u32(arena_t *arena, uint32_t value)
{
    ast_node_t *node_literal = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_literal);

    node_literal->kind = AST_NODE_LITERAL;
    node_literal->data.as_literal.kind = AST_LITERAL_U32;
    node_literal->data.as_literal.value.as_u32 = value;

    return node_literal;
}

ast_node_t *
ast_new_node_return_stmt(arena_t *arena)
{
    ast_node_t *node_return_stmt = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_return_stmt);

    node_return_stmt->kind = AST_NODE_RETURN_STMT;

    return node_return_stmt;
}

ast_node_t *
ast_new_node_block(arena_t *arena)
{
    ast_node_t *node_block = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_block);

    node_block->kind = AST_NODE_BLOCK;

    node_block->data.as_block.nodes = (list_t *)arena_alloc(arena, sizeof(list_t));
    assert(node_block->data.as_block.nodes);

    list_init(node_block->data.as_block.nodes, arena);

    return node_block;
}

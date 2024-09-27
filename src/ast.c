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
ast_new_translation_unit(arena_t *arena)
{
    ast_node_t *node = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node);

    node->kind = AST_NODE_TRANSLATION_UNIT;
    ast_translation_unit_t *translation_unit = &node->as_translation_unit;

    translation_unit->decls = (list_t *)arena_alloc(arena, sizeof(list_t));
    assert(translation_unit->decls);

    list_init(translation_unit->decls, arena);

    return node;
}

ast_node_t *
ast_new_node_fn_def(arena_t *arena, string_view_t id, list_t *params, string_view_t return_type, ast_node_t *block)
{
    assert(arena);
    assert(params);
    assert(block);

    ast_node_t *node_fn_def = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_fn_def);

    node_fn_def->kind = AST_NODE_FN_DEF;
    ast_fn_definition_t *fn_def = &node_fn_def->as_fn_def;

    fn_def->id = id;
    fn_def->return_type = return_type;
    fn_def->block = block;
    fn_def->params = params;

    return node_fn_def;
}

ast_node_t *
ast_new_node_fn_call(arena_t *arena, string_view_t id, list_t *args)
{
    assert(arena);
    assert(args);

    ast_node_t *node_fn_call = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_fn_call);

    node_fn_call->kind = AST_NODE_FN_CALL;
    ast_fn_call_t *fn_call = &node_fn_call->as_fn_call;

    fn_call->id = id;
    fn_call->args = args;

    return node_fn_call;
}

ast_node_t *
ast_new_node_var_def(arena_t *arena, string_view_t id, string_view_t type, ast_node_t *value)
{
    ast_node_t *node_var_def = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_var_def);

    node_var_def->kind = AST_NODE_VAR_DEF;
    ast_var_definition_t *var_def = &node_var_def->as_var_def;

    var_def->id = id;
    var_def->type = type;
    var_def->value = value;

    return node_var_def;
}

ast_node_t *
ast_new_node_bin_op(arena_t *arena, ast_binary_op_kind_t kind, ast_node_t *lhs, ast_node_t *rhs)
{
    ast_node_t *node_bin_op = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_bin_op);

    node_bin_op->kind = AST_NODE_BINARY_OP;
    node_bin_op->as_bin_op.kind = kind;
    node_bin_op->as_bin_op.lhs = lhs;
    node_bin_op->as_bin_op.rhs = rhs;

    return node_bin_op;
}

ast_node_t *
ast_new_node_literal_u32(arena_t *arena, uint32_t value)
{
    ast_node_t *node_literal = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_literal);

    node_literal->kind = AST_NODE_LITERAL;
    node_literal->as_literal.kind = AST_LITERAL_U32;
    node_literal->as_literal.as_u32 = value;

    return node_literal;
}

ast_node_t *
ast_new_node_ref(arena_t *arena, string_view_t id)
{
    ast_node_t *node_ref = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_ref);

    node_ref->kind = AST_NODE_REF;
    node_ref->as_ref.id = id;

    return node_ref;
}

ast_node_t *
ast_new_node_return_stmt(arena_t *arena, ast_node_t *expr)
{
    ast_node_t *node_return_stmt = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_return_stmt);

    node_return_stmt->kind = AST_NODE_RETURN_STMT;
    node_return_stmt->as_return_stmt.expr = expr;

    return node_return_stmt;
}

ast_node_t *
ast_new_node_if_stmt(arena_t *arena, ast_node_t *cond, ast_node_t *then, ast_node_t *_else)
{
    ast_node_t *node_if_stmt = arena_alloc(arena, sizeof(ast_node_t));
    assert(node_if_stmt);

    node_if_stmt->kind = AST_NODE_IF_STMT;
    node_if_stmt->as_if_stmt.cond = cond;
    node_if_stmt->as_if_stmt.then = then;
    node_if_stmt->as_if_stmt._else = _else;

    return node_if_stmt;
}

ast_node_t *
ast_new_node_block(arena_t *arena)
{
    ast_node_t *node_block = (ast_node_t *)arena_alloc(arena, sizeof(ast_node_t));
    assert(node_block);

    node_block->kind = AST_NODE_BLOCK;

    node_block->as_block.nodes = (list_t *)arena_alloc(arena, sizeof(list_t));
    assert(node_block->as_block.nodes);

    list_init(node_block->as_block.nodes, arena);

    return node_block;
}

ast_fn_param_t *
ast_new_fn_param(arena_t *arena, string_view_t id, string_view_t type_id)
{
    ast_fn_param_t *fn_param = (ast_fn_param_t *)arena_alloc(arena, sizeof(ast_fn_param_t));
    assert(fn_param);

    fn_param->id = id;
    fn_param->type_id = type_id;

    return fn_param;
}

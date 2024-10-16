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
#include "checker.h"
#include "scope.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>

static void
populate_scope(checker_t *checker, scope_t *scope, ast_node_t *ast);

checker_t *
checker_new(arena_t *arena)
{
    assert(arena);

    checker_t *checker = (checker_t *)arena_alloc(arena, sizeof(checker_t));
    if (checker == NULL) {
        fprintf(stderr,
                "[FATAL] Out of memory: checker_new: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    checker->arena = arena;
    return checker;
}

static type_t
type_from_id(string_view_t id)
{
    type_t type = { 0 };
    type.id = id;
    if (string_view_eq_to_cstr(id, "u8")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 1;
        type.as_primitive.kind = TYPE_U8;
        return type;
    }
    if (string_view_eq_to_cstr(id, "u16")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 2;
        type.as_primitive.kind = TYPE_U16;
        return type;
    }
    if (string_view_eq_to_cstr(id, "u32")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 4;
        type.as_primitive.kind = TYPE_U32;
        return type;
    }
    if (string_view_eq_to_cstr(id, "u64")) {
        type.kind = TYPE_PRIMITIVE;
        type.as_primitive.size = 8;
        type.as_primitive.kind = TYPE_U64;
        return type;
    }

    // FIXME: handle user defined types
    assert(0 && "unknown type");
}

/**
 * transform unknown types into actual types
 */
static void
type_resolve(type_t *type)
{
    switch (type->kind) {
        case TYPE_UNKNOWN:
            *type = type_from_id(type->as_unknown.id);
            break;
        case TYPE_PTR:
            type_resolve(type->as_ptr.type);
        case TYPE_PRIMITIVE:
            break;
    }
}

void
checker_check(checker_t *checker, ast_node_t *ast)
{
    assert(checker);
    assert(ast);

    scope_t *scope = scope_new(checker->arena);
    populate_scope(checker, scope, ast);

    // TODO: traverse the ast tree to verify semantics
}

static void
populate_scope(checker_t *checker, scope_t *scope, ast_node_t *ast)
{
    assert(checker);
    assert(scope);

    switch (ast->kind) {
        case AST_NODE_TRANSLATION_UNIT: {
            list_item_t *item = list_head(ast->as_translation_unit.decls);

            while (item != NULL) {
                populate_scope(checker, scope, (ast_node_t *)item->value);
                item = list_next(item);
            }
            return;
        }

        case AST_NODE_FN_DEF: {
            ast_fn_definition_t *fn_def = &ast->as_fn_def;
            fn_def->scope = scope_push(scope);

            type_resolve(fn_def->return_type);
            symbol_t *symbol =
                symbol_new(checker->arena, fn_def->id, fn_def->return_type);
            scope_insert(scope, symbol);

            list_item_t *item = list_head(fn_def->params);

            while (item != NULL) {
                ast_fn_param_t *param = (ast_fn_param_t *)item->value;

                type_resolve(param->type);
                symbol_t *symbol =
                    symbol_new(checker->arena, param->id, param->type);
                scope_insert(fn_def->scope, symbol);

                item = list_next(item);
            }

            if (ast->as_fn_def.block != NULL) {
                populate_scope(checker, fn_def->scope, ast->as_fn_def.block);
            }
            return;
        }

        case AST_NODE_FN_CALL: {
            ast->as_fn_call.scope = scope;

            list_item_t *item = list_head(ast->as_fn_call.args);

            while (item != NULL) {
                populate_scope(checker, scope, (ast_node_t *)item->value);
                item = list_next(item);
            }

            return;
        }

        case AST_NODE_IF_STMT: {
            populate_scope(checker, scope, ast->as_if_stmt.cond);
            populate_scope(checker, scope, ast->as_if_stmt.then);

            if (ast->as_if_stmt._else) {
                populate_scope(checker, scope, ast->as_if_stmt._else);
            }

            return;
        }

        case AST_NODE_WHILE_STMT: {
            populate_scope(checker, scope, ast->as_while_stmt.cond);
            populate_scope(checker, scope, ast->as_while_stmt.then);

            return;
        }

        case AST_NODE_BINARY_OP: {
            ast_binary_op_t bin_op = ast->as_bin_op;

            populate_scope(checker, scope, bin_op.lhs);
            populate_scope(checker, scope, bin_op.rhs);
            return;
        }

        case AST_NODE_UNARY_OP: {
            ast_unary_op_t unary_op = ast->as_unary_op;

            populate_scope(checker, scope, unary_op.expr);
            return;
        }

        case AST_NODE_RETURN_STMT: {
            ast_return_stmt_t return_stmt = ast->as_return_stmt;

            populate_scope(checker, scope, return_stmt.expr);
            return;
        }

        case AST_NODE_BLOCK: {
            ast_block_t block = ast->as_block;
            scope = scope_push(scope);

            list_item_t *item = list_head(block.nodes);

            while (item != NULL) {
                populate_scope(checker, scope, (ast_node_t *)item->value);
                item = list_next(item);
            }

            return;
        }

        case AST_NODE_VAR_DEF: {
            string_view_t id = ast->as_var_def.id;

            type_resolve(ast->as_var_def.type);

            symbol_t *symbol =
                symbol_new(checker->arena, id, ast->as_var_def.type);

            scope_insert(scope, symbol);
            ast->as_var_def.scope = scope;

            populate_scope(checker, scope, ast->as_var_def.value);
            return;
        }

        case AST_NODE_REF: {
            ast->as_ref.scope = scope;
            return;
        }

        case AST_NODE_LITERAL:
        case AST_NODE_UNKNOWN:
            return;
    }
}

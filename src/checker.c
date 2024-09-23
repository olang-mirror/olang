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
        fprintf(stderr, "[FATAL] Out of memory: checker_new: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    checker->arena = arena;
    return checker;
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
            ast->as_fn_def.scope = scope;
            // FIXME: insert function symbol to scope
            populate_scope(checker, scope, ast->as_fn_def.block);
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

        case AST_NODE_BINARY_OP: {
            ast_binary_op_t bin_op = ast->as_bin_op;

            populate_scope(checker, scope, bin_op.lhs);
            populate_scope(checker, scope, bin_op.rhs);
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

            symbol_t *symbol = symbol_new(checker->arena, id, type_from_id(ast->as_var_def.type));

            scope_insert(scope, symbol);
            ast->as_var_def.scope = scope;
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

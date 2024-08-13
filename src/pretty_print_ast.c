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
#include "pretty_print_ast.h"
#include "arena.h"
#include "list.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET "\x1b[0m"

// clang-format off
#define PP_IS_BIT_SET(data, index) ((data) & 1 << index)
// clang-format on

typedef struct pretty_print_node
{
    char *name;
    list_t *children;
} pretty_print_node_t;

static bool
stdout_supports_color()
{
    struct stat st;
    fstat(STDOUT_FILENO, &st);
    return S_ISCHR(st.st_mode);
}

static void
pretty_print_print_ident(uint64_t *prefix, size_t level, bool lst_children)
{
    assert(level < 64);

    bool support_color = stdout_supports_color();

    if (support_color) {
        printf(ANSI_COLOR_MAGENTA);
    }

    for (size_t i = 0; i < level; ++i) {

        if (!PP_IS_BIT_SET(*prefix, i)) {
            printf("  ");
            continue;
        }

        bool last_index = i + 1 == level;

        if (!last_index) {
            printf("| ");
        } else if (lst_children) {
            printf("`-");
        } else {
            printf("|-");
        }
    }

    if (support_color) {
        printf(ANSI_COLOR_RESET);
    }
}

static void
pretty_print_tree(pretty_print_node_t *node, uint64_t *prefix, size_t level, bool lst_children)
{
    pretty_print_print_ident(prefix, level, lst_children);

    list_t *list = node->children;
    if (list != NULL)
        (*prefix) |= 1 << level;
    if (lst_children)
        (*prefix) ^= 1 << (level - 1);

    printf("%s\n", node->name);

    size_t size = list_size(list);
    for (size_t i = 0; i < size; ++i) {
        pretty_print_node_t *it = (pretty_print_node_t *)list_get(list, i)->value;
        pretty_print_tree(it, prefix, level + 1, i + 1 == size);
    }
}

static pretty_print_node_t *
pretty_print_node_new(arena_t *arena)
{
    pretty_print_node_t *node = (pretty_print_node_t *)arena_alloc(arena, sizeof(pretty_print_node_t));
    node->children = (list_t *)arena_alloc(arena, sizeof(list_t));
    list_init(node->children, arena);
    return node;
}

static pretty_print_node_t *
ast_node_to_pretty_print_node(ast_node_t *ast, arena_t *arena)
{
    switch (ast->kind) {
        case AST_NODE_PROGRAM: {
            pretty_print_node_t *node = pretty_print_node_new(arena);
            node->name = "Translation_Unit";

            pretty_print_node_t *fn_node = ast_node_to_pretty_print_node(ast->data.as_program.fn, arena);
            list_append(node->children, fn_node);
            return node;
        }
        case AST_NODE_FN_DEF: {
            pretty_print_node_t *node = pretty_print_node_new(arena);
            ast_fn_definition_t fn_def = ast->data.as_fn_def;

            char name[256];
            sprintf(name,
                    "Function_Definition <name:" SV_FMT "> <return:%d>",
                    SV_ARG(fn_def.identifier),
                    fn_def.return_type);
            node->name = (char *)arena_alloc(arena, sizeof(char) * (strlen(name) + 1));
            strcpy(node->name, name);

            pretty_print_node_t *block = ast_node_to_pretty_print_node(fn_def.block, arena);
            list_append(node->children, block);
            return node;
        }
        case AST_NODE_BLOCK: {
            pretty_print_node_t *node = pretty_print_node_new(arena);
            ast_block_t block = ast->data.as_block;

            node->name = "Block";

            size_t block_nodes_size = list_size(block.nodes);
            for (size_t i = 0; i < block_nodes_size; ++i) {
                ast_node_t *ast_node = (ast_node_t *)list_get(block.nodes, i)->value;
                pretty_print_node_t *child = ast_node_to_pretty_print_node(ast_node, arena);
                list_append(node->children, child);
            }
            return node;
        }
        case AST_NODE_RETURN_STMT: {
            pretty_print_node_t *node = pretty_print_node_new(arena);
            ast_return_stmt_t return_stmt = ast->data.as_return_stmt;

            node->name = "Return_Statement";

            pretty_print_node_t *child = ast_node_to_pretty_print_node(return_stmt.data, arena);
            list_append(node->children, child);

            return node;
        }
        case AST_NODE_LITERAL: {
            pretty_print_node_t *node = pretty_print_node_new(arena);
            ast_literal_t literal = ast->data.as_literal;

            char name[256];
            switch (literal.kind) {
                case AST_LITERAL_U32: {
                    sprintf(name, "Literal <kind:u32> <value:%d>", literal.as_u32);
                    node->name = (char *)arena_alloc(arena, sizeof(char) * (strlen(name) + 1));
                    strcpy(node->name, name);
                    break;
                }
                default:
                    assert(0 && "literal not implemented");
            }

            return node;
        }
        case AST_NODE_BINARY_OP: {
            pretty_print_node_t *node = pretty_print_node_new(arena);
            ast_binary_op_t binop = ast->data.as_bin_op;

            switch (binop.kind) {
                case AST_BINOP_ADDITION: {
                    node->name = "Binary_Operation (+)";
                    break;
                }
                case AST_BINOP_SUBTRACTION: {
                    node->name = "Binary_Operation (-)";
                    break;
                }
                case AST_BINOP_MULTIPLICATION: {
                    node->name = "Binary_Operation (*)";
                    break;
                }
                case AST_BINOP_DIVISION: {
                    node->name = "Binary_Operation (/)";
                    break;
                }
                case AST_BINOP_REMINDER: {
                    node->name = "Binary_Operation (%)";
                    break;
                }
                case AST_BINOP_BITWISE_LSHIFT: {
                    node->name = "Binary_Operation (<<)";
                    break;
                }
                case AST_BINOP_BITWISE_RSHIFT: {
                    node->name = "Binary_Operation (>>)";
                    break;
                }
                case AST_BINOP_BITWISE_XOR: {
                    node->name = "Binary_Operation (^)";
                    break;
                }
                case AST_BINOP_BITWISE_AND: {
                    node->name = "Binary_Operation (&)";
                    break;
                }
                case AST_BINOP_BITWISE_OR: {
                    node->name = "Binary_Operation (|)";
                    break;
                }
                case AST_BINOP_CMP_LT: {
                    node->name = "Binary_Operation (<)";
                    break;
                }
                case AST_BINOP_CMP_GT: {
                    node->name = "Binary_Operation (>)";
                    break;
                }
                case AST_BINOP_CMP_LEQ: {
                    node->name = "Binary_Operation (<=)";
                    break;
                }
                case AST_BINOP_CMP_GEQ: {
                    node->name = "Binary_Operation (>=)";
                    break;
                }
                case AST_BINOP_CMP_EQ: {
                    node->name = "Binary_Operation (==)";
                    break;
                }
                case AST_BINOP_CMP_NEQ: {
                    node->name = "Binary_Operation (!=)";
                    break;
                }
                case AST_BINOP_LOGICAL_AND: {
                    node->name = "Binary_Operation (&&)";
                    break;
                }
                case AST_BINOP_LOGICAL_OR: {
                    node->name = "Binary_Operation (|)";
                    break;
                }
                default:
                    assert(false && "binop not implemented");
            }

            pretty_print_node_t *lhs = ast_node_to_pretty_print_node(binop.lhs, arena);
            pretty_print_node_t *rhs = ast_node_to_pretty_print_node(binop.rhs, arena);

            list_append(node->children, lhs);
            list_append(node->children, rhs);

            return node;
        }
        default: {
            printf("node kind = '%d' not implmented\n", ast->kind);
            assert(false);
        }
    }
    return NULL;
}

void
pretty_print_ast(ast_node_t *ast)
{
    arena_t arena = arena_new(8 * 1024);
    pretty_print_node_t *root = ast_node_to_pretty_print_node(ast, &arena);
    uint64_t prefix = 0;

    pretty_print_tree(root, &prefix, 0, true);

    arena_free(&arena);
}

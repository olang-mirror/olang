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
#include <stdio.h>

#include "codegen_linux_x86_64.h"
#include "list.h"

#define SYS_exit (60)

static void
codegen_linux_x86_64_emit_start_entrypoint(FILE *out);

static void
codegen_linux_x86_64_emit_function(FILE *out, ast_fn_definition_t *fn);

void
codegen_linux_x86_64_emit_program(FILE *out, ast_node_t *node)
{
    codegen_linux_x86_64_emit_start_entrypoint(out);

    assert(node->kind == AST_NODE_PROGRAM);
    ast_program_t program = node->as_program;

    ast_fn_definition_t fn = program.fn->as_fn_def;

    assert(string_view_eq_to_cstr(fn.identifier, "main"));
    codegen_linux_x86_64_emit_function(out, &fn);
}

static void
codegen_linux_x86_64_emit_start_entrypoint(FILE *out)
{
    fprintf(out, ".text\n");
    fprintf(out, ".globl _start\n\n");

    fprintf(out, "_start:\n");
    fprintf(out, "    call main\n");
    fprintf(out, "    mov %%eax, %%edi\n");
    fprintf(out, "    mov $%d, %%eax\n", SYS_exit);
    fprintf(out, "    syscall\n");
}

static void
codegen_linux_x86_64_emit_function(FILE *out, ast_fn_definition_t *fn)
{
    ast_node_t *block_node = fn->block;
    assert(block_node->kind == AST_NODE_BLOCK);
    ast_block_t block = block_node->as_block;

    assert(list_size(block.nodes) == 1);

    list_item_t *nodes_item = list_get(block.nodes, 0);
    ast_node_t *return_node = nodes_item->value;
    assert(return_node->kind == AST_NODE_RETURN_STMT);
    ast_return_stmt_t return_stmt = return_node->as_return_stmt;

    ast_node_t *literal_node = return_stmt.data;
    assert(literal_node->kind == AST_NODE_LITERAL);
    ast_literal_t literal_u32 = literal_node->as_literal;

    assert(literal_u32.kind == AST_LITERAL_U32);
    uint32_t exit_code = literal_u32.as_u32;

    fprintf(out, "" SV_FMT ":\n", SV_ARG(fn->identifier));
    fprintf(out, "    mov $%d, %%eax\n", exit_code);
    fprintf(out, "    ret\n");
}

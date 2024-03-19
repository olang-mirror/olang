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

#include "codegen_linux_aarch64.h"
#include "list.h"

#define SYS_exit (93)

/**
 *  ───────────────────────────────────────────────────────────────────
 *  Arch/ABI    Instruction           System  Ret  Ret  Error    Notes
 *                                    call #  val  val2
 *  ───────────────────────────────────────────────────────────────────
 *  arm64       svc #0                w8      x0   x1   -
 *  ──────────────────────────────────────────────────────────────
 *  Arch/ABI      arg1  arg2  arg3  arg4  arg5  arg6  arg7  Notes
 *  ──────────────────────────────────────────────────────────────
 *  arm64         x0    x1    x2    x3    x4    x5    -
 */

static void
codegen_linux_aarch64_emit_start_entrypoint(FILE *out);

static void
codegen_linux_aarch64_emit_function(FILE *out, ast_fn_definition_t *fn);

void
codegen_linux_aarch64_emit_program(FILE *out, ast_node_t *node)
{
    codegen_linux_aarch64_emit_start_entrypoint(out);

    assert(node->kind == AST_NODE_PROGRAM);
    ast_program_t program = node->data.as_program;

    ast_fn_definition_t fn = program.fn->data.as_fn_def;

    assert(string_view_eq_to_cstr(fn.identifier, "main"));
    codegen_linux_aarch64_emit_function(out, &fn);
}

static void
codegen_linux_aarch64_emit_start_entrypoint(FILE *out)
{
    fprintf(out, ".text\n");
    fprintf(out, ".globl _start\n\n");

    fprintf(out, "_start:\n");
    fprintf(out, "    bl main\n");
    fprintf(out, "    mov w8, #%d\n", SYS_exit);
    fprintf(out, "    svc #0\n");
}

static void
codegen_linux_aarch64_emit_function(FILE *out, ast_fn_definition_t *fn)
{
    ast_node_t *block_node = fn->block;
    assert(block_node->kind == AST_NODE_BLOCK);
    ast_block_t block = block_node->data.as_block;

    assert(list_size(block.nodes) == 1);

    list_item_t *nodes_item = list_get(block.nodes, 0);
    ast_node_t *return_node = nodes_item->value;
    assert(return_node->kind == AST_NODE_RETURN_STMT);
    ast_return_stmt_t return_stmt = return_node->data.as_return_stmt;

    ast_node_t *literal_node = return_stmt.data;
    assert(literal_node->kind == AST_NODE_LITERAL);
    ast_literal_t literal_u32 = literal_node->data.as_literal;

    assert(literal_u32.kind == AST_LITERAL_U32);
    uint32_t exit_code = literal_u32.value.as_u32;

    fprintf(out, "" SV_FMT ":\n", SV_ARG(fn->identifier));
    fprintf(out, "    mov x0, #%d\n", exit_code);
    fprintf(out, "    ret\n");
}

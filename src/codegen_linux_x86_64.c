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
#include "map.h"
#include "scope.h"

#define SYS_exit (60)
#define PTR_HEX_CSTR_SIZE (16 + 1)

// The call instruction pushes EIP into stack so the first 8 bytes from stack
// must be preserved else the ret instruction will jump to nowere.
#define X86_CALL_EIP_STACK_OFFSET (8)
#define X86_CALL_ARG_SIZE 6

#define bytes_max(a, b) ((a) > (b) ? (a) : (b))

typedef enum x86_64_register_type
{
    REG_ACCUMULATOR,
    REG_BASE,
    REG_COUNTER,
    REG_DATA,
    REG_SRC_IDX,
    REG_DEST_IDX,
    REG_STACK_PTR,
    REG_BASE_PTR,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15
} x86_64_register_type_t;

/**
 * Arch/ABI      arg1  arg2  arg3  arg4  arg5  arg6  arg7  Notes
 * ──────────────────────────────────────────────────────────────
 * x86-64        rdi   rsi   rdx   r10   r8    r9    -
 */
static int x86_call_args[X86_CALL_ARG_SIZE] = { REG_DEST_IDX, REG_SRC_IDX,
                                                REG_DATA,     REG_R10,
                                                REG_R8,       REG_R9 };

static void
codegen_linux_x86_64_emit_start_entrypoint(codegen_x86_64_t *codegen);

static void
codegen_linux_x86_64_emit_function(codegen_x86_64_t *codegen,
                                   ast_fn_definition_t *fn);

static void
codegen_linux_x86_64_emit_if(codegen_x86_64_t *codegen, ast_if_stmt_t is_stmt);

static void
codegen_linux_x86_64_put_stack_offset(codegen_x86_64_t *codegen,
                                      symbol_t *symbol,
                                      size_t offset);

static size_t
codegen_linux_x86_64_get_stack_offset(codegen_x86_64_t *codegen,
                                      symbol_t *symbol);

static size_t
type_to_bytes(type_t *type);

static char *
get_reg_for(x86_64_register_type_t type, size_t bytes);

void
codegen_linux_x86_64_init(codegen_x86_64_t *codegen, arena_t *arena, FILE *out)
{
    assert(codegen);
    assert(arena);
    assert(codegen);
    codegen->base_offset = 0;
    codegen->symbols_stack_offset = map_new(arena);
    codegen->out = out;
    codegen->arena = arena;
}

void
codegen_linux_x86_64_emit_translation_unit(codegen_x86_64_t *codegen,
                                           ast_node_t *node)
{
    codegen->label_index = 0;
    codegen_linux_x86_64_emit_start_entrypoint(codegen);

    assert(node->kind == AST_NODE_TRANSLATION_UNIT);
    ast_translation_unit_t translation_unit = node->as_translation_unit;

    list_item_t *item = list_head(translation_unit.decls);

    bool main_found = false;

    while (item != NULL) {
        ast_node_t *decl = (ast_node_t *)item->value;

        if (decl->kind == AST_NODE_FN_DEF) {
            ast_fn_definition_t fn = decl->as_fn_def;
            codegen_linux_x86_64_emit_function(codegen, &fn);

            main_found = main_found || string_view_eq_to_cstr(fn.id, "main");
        } else {
            assert(0 && "translation unit only supports function declarations");
        }

        item = list_next(item);
    }

    assert(main_found && "main function is required.");
}

static void
codegen_linux_x86_64_emit_start_entrypoint(codegen_x86_64_t *codegen)
{
    fprintf(codegen->out, ".text\n");
    fprintf(codegen->out, ".globl _start\n\n");

    fprintf(codegen->out, "_start:\n");
    fprintf(codegen->out, "    call main\n");
    fprintf(codegen->out, "    mov %%eax, %%edi\n");
    fprintf(codegen->out, "    mov $%d, %%eax\n", SYS_exit);
    fprintf(codegen->out, "    syscall\n");
}

static size_t
codegen_linux_x86_64_get_next_label(codegen_x86_64_t *codegen)
{
    return ++codegen->label_index;
}

typedef size_t size_in_bytes_t;

static size_in_bytes_t
codegen_linux_x86_64_emit_expression(codegen_x86_64_t *codegen,
                                     ast_node_t *expr_node)
{
    switch (expr_node->kind) {
        case AST_NODE_LITERAL: {
            ast_literal_t literal_u32 = expr_node->as_literal;
            assert(literal_u32.kind == AST_LITERAL_U32);
            uint32_t n = literal_u32.as_u32;

            fprintf(codegen->out, "    mov $%d, %%eax\n", n);
            return 4;
        }
        case AST_NODE_REF: {
            ast_ref_t ref = expr_node->as_ref;

            symbol_t *symbol = scope_lookup(ref.scope, ref.id);
            assert(symbol);

            size_t offset =
                codegen_linux_x86_64_get_stack_offset(codegen, symbol);

            size_t bytes = type_to_bytes(symbol->type);

            fprintf(codegen->out,
                    "    mov -%ld(%%rbp), %s\n",
                    offset,
                    get_reg_for(REG_ACCUMULATOR, bytes));
            return bytes;
        }
        case AST_NODE_FN_CALL: {
            ast_fn_call_t fn_call = expr_node->as_fn_call;

            symbol_t *symbol = scope_lookup(fn_call.scope, fn_call.id);
            assert(symbol);

            size_t i = 0;
            for (list_item_t *item = list_head(fn_call.args); item != NULL;
                 item = list_next(item)) {
                // FIXME: add support for more args than X86_CALL_ARG_SIZE
                assert(i < X86_CALL_ARG_SIZE);

                ast_node_t *arg_node = (ast_node_t *)item->value;

                codegen_linux_x86_64_emit_expression(codegen, arg_node);

                fprintf(codegen->out,
                        "    push %s\n",
                        get_reg_for(REG_ACCUMULATOR, 8));
                ++i;
            }

            for (; i > 0; --i) {
                fprintf(codegen->out,
                        "    pop %s\n",
                        get_reg_for(x86_call_args[i - 1], 8));
            }

            fprintf(codegen->out, "    call " SV_FMT "\n", SV_ARG(fn_call.id));

            return type_to_bytes(symbol->type);
        }
        case AST_NODE_BINARY_OP: {
            ast_binary_op_t bin_op = expr_node->as_bin_op;
            switch (bin_op.kind) {
                case AST_BINOP_ADDITION: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    add %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_MULTIPLICATION: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    mul %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_DIVISION: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    xor %%rdx, %%rdx\n");
                    fprintf(codegen->out,
                            "    div %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_REMINDER: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    xor %%rdx, %%rdx\n");
                    fprintf(codegen->out,
                            "    div %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes));
                    fprintf(codegen->out,
                            "    mov %s, %s\n",
                            get_reg_for(REG_DATA, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_SUBTRACTION: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    sub %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_CMP_EQ: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    cmp %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));
                    fprintf(codegen->out, "    sete %%al\n");
                    fprintf(codegen->out,
                            "    movzb %%al, %s\n",
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_CMP_LT: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    cmp %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));
                    fprintf(codegen->out, "    setl %%al\n");
                    fprintf(codegen->out,
                            "    movzb %%al, %s\n",
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_CMP_GT: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    cmp %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));
                    fprintf(codegen->out, "    setg %%al\n");
                    fprintf(codegen->out,
                            "    movzb %%al, %s\n",
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_CMP_NEQ: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    cmp %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));
                    fprintf(codegen->out, "    setne %%al\n");
                    fprintf(codegen->out,
                            "    movzb %%al, %s\n",
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_CMP_LEQ: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    cmp %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));
                    fprintf(codegen->out, "    setle %%al\n");
                    fprintf(codegen->out,
                            "    movzb %%al, %s\n",
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_CMP_GEQ: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    cmp %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));
                    fprintf(codegen->out, "    setge %%al\n");
                    fprintf(codegen->out,
                            "    movzb %%al, %s\n",
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_BITWISE_LSHIFT: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    shl %%cl, %s\n",
                            get_reg_for(REG_ACCUMULATOR, lhs_bytes));

                    return lhs_bytes;
                }
                case AST_BINOP_BITWISE_RSHIFT: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    shr %%cl, %s\n",
                            get_reg_for(REG_ACCUMULATOR, lhs_bytes));

                    return lhs_bytes;
                }
                case AST_BINOP_BITWISE_XOR: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    xor %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_BITWISE_AND: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    and %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_BITWISE_OR: {
                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);

                    size_in_bytes_t expr_bytes =
                        bytes_max(rhs_bytes, lhs_bytes);

                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out,
                            "    or %s, %s\n",
                            get_reg_for(REG_COUNTER, expr_bytes),
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_BINOP_LOGICAL_AND: {
                    size_t label_exit =
                        codegen_linux_x86_64_get_next_label(codegen);

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);
                    fprintf(codegen->out,
                            "    cmp $0, %s\n",
                            get_reg_for(REG_ACCUMULATOR, lhs_bytes));
                    fprintf(codegen->out, "    je .L%ld\n", label_exit);

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out,
                            "    cmp $0, %s\n",
                            get_reg_for(REG_ACCUMULATOR, rhs_bytes));
                    fprintf(codegen->out, "    je .L%ld\n", label_exit);
                    fprintf(codegen->out, "    mov $1, %%rax\n");
                    fprintf(codegen->out, ".L%ld:\n", label_exit);

                    return 1;
                }
                case AST_BINOP_LOGICAL_OR: {
                    size_t label_t =
                        codegen_linux_x86_64_get_next_label(codegen);
                    size_t label_f =
                        codegen_linux_x86_64_get_next_label(codegen);

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t lhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.lhs);
                    fprintf(codegen->out,
                            "    cmp $0, %s\n",
                            get_reg_for(REG_ACCUMULATOR, lhs_bytes));
                    fprintf(codegen->out, "    jne .L%ld\n", label_t);

                    fprintf(codegen->out, "    xor %%rax, %%rax\n");
                    size_in_bytes_t rhs_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             bin_op.rhs);
                    fprintf(codegen->out,
                            "    cmp $0, %s\n",
                            get_reg_for(REG_ACCUMULATOR, rhs_bytes));
                    fprintf(codegen->out, "    je .L%ld\n", label_f);

                    fprintf(codegen->out, ".L%ld:\n", label_t);
                    fprintf(codegen->out, "    mov $1, %%rax\n");
                    fprintf(codegen->out, ".L%ld:\n", label_f);

                    return 1;
                }
                case AST_BINOP_ASSIGN: {
                    // FIXME: It may not be a ref
                    ast_ref_t ref = bin_op.lhs->as_ref;
                    scope_t *scope = ref.scope;

                    symbol_t *symbol = scope_lookup(scope, ref.id);
                    assert(symbol);

                    size_t offset =
                        codegen_linux_x86_64_get_stack_offset(codegen, symbol);

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);

                    size_t type_size = type_to_bytes(symbol->type);
                    fprintf(codegen->out,
                            "    mov %s, -%ld(%%rbp)\n",
                            get_reg_for(REG_ACCUMULATOR, type_size),
                            offset);

                    // FIXME: we don't support a = b = c
                    return 0;
                }
                default: {
                    assert(0 && "unsupported binary operation");
                    return 0;
                }
            }
        }

        case AST_NODE_UNARY_OP: {
            ast_unary_op_t unary_op = expr_node->as_unary_op;
            switch (unary_op.kind) {
                case AST_UNARY_BITWISE_NOT: {
                    size_in_bytes_t expr_bytes =
                        codegen_linux_x86_64_emit_expression(codegen,
                                                             unary_op.expr);

                    fprintf(codegen->out,
                            "    not %s\n",
                            get_reg_for(REG_ACCUMULATOR, expr_bytes));

                    return expr_bytes;
                }
                case AST_UNARY_ADDRESSOF: {
                    assert(unary_op.expr->kind == AST_NODE_REF &&
                           "unsupported unary expression for addressof (&)");

                    ast_ref_t ref = unary_op.expr->as_ref;

                    symbol_t *symbol = scope_lookup(ref.scope, ref.id);
                    assert(symbol);

                    size_t offset =
                        codegen_linux_x86_64_get_stack_offset(codegen, symbol);

                    fprintf(
                        codegen->out, "    lea -%ld(%%rbp), %%rax\n", offset);
                    return 8;
                }
                default: {
                    assert(0 && "unsupported unary operation");
                    return 0;
                }
            }
        }

        default:
            assert(0 && "unsupported expression");
    }
}
static void
codegen_linux_x86_64_emit_block(codegen_x86_64_t *codegen, ast_block_t *block)
{
    size_t block_offset = codegen->base_offset;
    size_t nodes_len = list_size(block->nodes);

    for (size_t i = 0; i < nodes_len; ++i) {
        ast_node_t *node = list_get(block->nodes, i)->value;
        switch (node->kind) {
            case AST_NODE_RETURN_STMT: {
                ast_return_stmt_t return_stmt = node->as_return_stmt;

                ast_node_t *expr = return_stmt.expr;

                codegen_linux_x86_64_emit_expression(codegen, expr);

                fprintf(codegen->out, "    mov %%rbp, %%rsp\n");
                fprintf(codegen->out, "    pop %%rbp\n");
                fprintf(codegen->out, "    ret\n");

                break;
            }

            case AST_NODE_VAR_DEF: {
                ast_var_definition_t var_def = node->as_var_def;
                scope_t *scope = var_def.scope;

                symbol_t *symbol = scope_lookup(scope, var_def.id);
                assert(symbol);

                codegen_linux_x86_64_put_stack_offset(
                    codegen, symbol, codegen->base_offset);

                if (var_def.value) {
                    codegen_linux_x86_64_emit_expression(codegen,
                                                         var_def.value);
                }

                size_t type_size = type_to_bytes(symbol->type);

                fprintf(codegen->out,
                        "    mov %s, -%ld(%%rbp)\n",
                        get_reg_for(REG_ACCUMULATOR, type_size),
                        codegen->base_offset);
                codegen->base_offset += type_size;

                break;
            }

            case AST_NODE_BINARY_OP: {
                codegen_linux_x86_64_emit_expression(codegen, node);
                break;
            }

            case AST_NODE_IF_STMT: {
                codegen_linux_x86_64_emit_if(codegen, node->as_if_stmt);
                break;
            }

            case AST_NODE_WHILE_STMT: {
                ast_while_stmt_t while_stmt = node->as_while_stmt;

                ast_node_t *cond = while_stmt.cond;
                ast_node_t *then = while_stmt.then;

                size_t begin_label =
                    codegen_linux_x86_64_get_next_label(codegen);
                size_t end_label = codegen_linux_x86_64_get_next_label(codegen);

                fprintf(codegen->out, ".L%ld:\n", begin_label);
                codegen_linux_x86_64_emit_expression(codegen, cond);
                fprintf(codegen->out, "    cmp $1, %%rax\n");
                fprintf(codegen->out, "    jnz .L%ld\n", end_label);

                assert(then->kind == AST_NODE_BLOCK &&
                       "invalid while-then block");
                ast_block_t then_block = then->as_block;

                codegen_linux_x86_64_emit_block(codegen, &then_block);

                fprintf(codegen->out, "    jmp .L%ld\n", begin_label);
                fprintf(codegen->out, ".L%ld:\n", end_label);

                break;
            }
            default: {
                // FIXME: improve error: replace the node->kind to a string
                // representation
                fprintf(stderr, "node kind %d not supported\n", node->kind);
                assert(0 && "unsupported block statement");
                break;
            }
        }
    }

    codegen->base_offset = block_offset;
}

static void
codegen_linux_x86_64_emit_if(codegen_x86_64_t *codegen, ast_if_stmt_t if_stmt)
{
    ast_node_t *cond = if_stmt.cond;
    ast_node_t *then = if_stmt.then;
    ast_node_t *_else = if_stmt._else;

    size_t end_if_label = codegen_linux_x86_64_get_next_label(codegen);
    size_t end_else_label = codegen_linux_x86_64_get_next_label(codegen);

    codegen_linux_x86_64_emit_expression(codegen, cond);
    fprintf(codegen->out, "    cmp $1, %%rax\n");
    fprintf(codegen->out, "    jnz .L%ld\n", end_if_label);

    assert(then->kind == AST_NODE_BLOCK && "invalid if-then block");
    ast_block_t then_block = then->as_block;

    codegen_linux_x86_64_emit_block(codegen, &then_block);
    fprintf(codegen->out, "    jmp .L%ld\n", end_else_label);

    fprintf(codegen->out, ".L%ld:\n", end_if_label);

    if (_else != NULL) {
        if (_else->kind == AST_NODE_IF_STMT) {
            ast_if_stmt_t else_if = _else->as_if_stmt;
            codegen_linux_x86_64_emit_if(codegen, else_if);
        } else {
            ast_block_t else_block = _else->as_block;
            codegen_linux_x86_64_emit_block(codegen, &else_block);
        }
    }

    fprintf(codegen->out, ".L%ld:\n", end_else_label);
}

static size_t
type_to_bytes(type_t *type)
{
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            return type->as_primitive.size;
        }
        case TYPE_PTR: {
            return 8;
        }
        case TYPE_UNKNOWN: {
            assert(0 && "cannot calculate size of an unknown type: probably a "
                        "parser issue.");
        }
    }

    assert(0 && "unreachable");
}

static size_t
calculate_fn_local_size(scope_t *scope)
{
    assert(scope);

    // The local_size starts with 8 bytes since the first 8 bytes from the
    // stack are reserved to store RBP during the prelude
    size_t local_size = 8;

    map_kv_t *kvs[scope->symbols->size];

    map_get_kvs(scope->symbols, kvs);

    for (size_t i = 0; i < scope->symbols->size; ++i) {
        symbol_t *symbol = (symbol_t *)kvs[i]->value;
        local_size += type_to_bytes(symbol->type);
    }

    size_t max_child_local_size = 0;

    list_item_t *item = list_head(scope->children);

    while (item != NULL) {
        size_t child_local_size =
            calculate_fn_local_size((scope_t *)item->value);

        if (child_local_size > max_child_local_size) {
            max_child_local_size = child_local_size;
        }

        item = list_next(item);
    }

    return local_size + max_child_local_size;
}

static void
codegen_linux_x86_64_emit_function(codegen_x86_64_t *codegen,
                                   ast_fn_definition_t *fn_def)
{
    codegen->base_offset = X86_CALL_EIP_STACK_OFFSET;

    ast_node_t *block_node = fn_def->block;
    fprintf(codegen->out, "" SV_FMT ":\n", SV_ARG(fn_def->id));

    fprintf(codegen->out, "    push %%rbp\n");
    fprintf(codegen->out, "    mov %%rsp, %%rbp\n");

    size_t i = 0;
    for (list_item_t *item = list_head(fn_def->params); item != NULL;
         item = list_next(item)) {
        assert(i < X86_CALL_ARG_SIZE);

        ast_fn_param_t *param = item->value;

        symbol_t *symbol = scope_lookup(fn_def->scope, param->id);
        assert(symbol);

        size_t offset = codegen->base_offset;

        codegen_linux_x86_64_put_stack_offset(
            codegen, symbol, codegen->base_offset);

        fprintf(codegen->out,
                "    mov %s, -%ld(%%rbp)\n",
                // FIXME: Type may not be an as_primitive
                get_reg_for(x86_call_args[i], symbol->type->as_primitive.size),
                offset);

        // FIXME: add offset according to the param size
        codegen->base_offset += 8;
        ++i;
    }

    size_t local_size = calculate_fn_local_size(fn_def->scope);

    if (local_size != 0) {
        fprintf(codegen->out, "    sub $%ld, %%rsp\n", local_size);
    }

    assert(block_node->kind == AST_NODE_BLOCK);
    ast_block_t block = block_node->as_block;

    codegen_linux_x86_64_emit_block(codegen, &block);
}

static void
codegen_linux_x86_64_put_stack_offset(codegen_x86_64_t *codegen,
                                      symbol_t *symbol,
                                      size_t offset)
{

    size_t *stack_offset = arena_alloc(codegen->arena, sizeof(size_t));
    *stack_offset = offset;

    char symbol_ptr[PTR_HEX_CSTR_SIZE];
    sprintf(symbol_ptr, "%lx", (uintptr_t)symbol);

    map_put(codegen->symbols_stack_offset, symbol_ptr, stack_offset);
}

static size_t
codegen_linux_x86_64_get_stack_offset(codegen_x86_64_t *codegen,
                                      symbol_t *symbol)
{
    char symbol_ptr[PTR_HEX_CSTR_SIZE];
    sprintf(symbol_ptr, "%lx", (uintptr_t)symbol);

    return *(size_t *)map_get(codegen->symbols_stack_offset, symbol_ptr);
}

static char *
get_reg_for(x86_64_register_type_t type, size_t bytes)
{
    switch (type) {
        case REG_ACCUMULATOR: {
            if (bytes <= 1) {
                return "%ah";
            } else if (bytes <= 2) {
                return "%ax";
            } else if (bytes <= 4) {
                return "%eax";
            }
            return "%rax";
        }
        case REG_BASE: {
            if (bytes <= 1) {
                return "%bh";
            } else if (bytes <= 2) {
                return "%bx";
            } else if (bytes <= 4) {
                return "%ebx";
            }
            return "%rbx";
        }
        case REG_COUNTER: {
            if (bytes <= 1) {
                return "%ch";
            } else if (bytes <= 2) {
                return "%cx";
            } else if (bytes <= 4) {
                return "%ecx";
            }
            return "%rcx";
        }
        case REG_DATA: {
            if (bytes <= 1) {
                return "%dh";
            } else if (bytes <= 2) {
                return "%dx";
            } else if (bytes <= 4) {
                return "%edx";
            }
            return "%rdx";
        }
        case REG_SRC_IDX: {
            if (bytes <= 1) {
                return "%sil";
            } else if (bytes <= 2) {
                return "%si";
            } else if (bytes <= 4) {
                return "%esi";
            }
            return "%rsi";
        }
        case REG_DEST_IDX: {
            if (bytes <= 1) {
                return "%sil";
            } else if (bytes <= 2) {
                return "%di";
            } else if (bytes <= 4) {
                return "%edi";
            }
            return "%rdi";
        }
        case REG_STACK_PTR: {
            if (bytes <= 1) {
                return "%spl";
            } else if (bytes <= 2) {
                return "%sp";
            } else if (bytes <= 4) {
                return "%esp";
            }
            return "%rsp";
        }
        case REG_BASE_PTR: {
            if (bytes <= 1) {
                return "%bpl";
            } else if (bytes <= 2) {
                return "%bp";
            } else if (bytes <= 4) {
                return "%ebp";
            }
            return "%rbp";
        }
        case REG_R8: {
            if (bytes <= 1) {
                return "%r8b";
            } else if (bytes <= 2) {
                return "%r8w";
            } else if (bytes <= 4) {
                return "%r8d";
            }
            return "%r8";
        }
        case REG_R9: {
            if (bytes <= 1) {
                return "%r9b";
            } else if (bytes <= 2) {
                return "%r9w";
            } else if (bytes <= 4) {
                return "%r9d";
            }
            return "%r9";
        }
        case REG_R10: {
            if (bytes <= 1) {
                return "%r10b";
            } else if (bytes <= 2) {
                return "%r10w";
            } else if (bytes <= 4) {
                return "%r10d";
            }
            return "%r10";
        }
        case REG_R11: {
            if (bytes <= 1) {
                return "%r11b";
            } else if (bytes <= 2) {
                return "%r11w";
            } else if (bytes <= 4) {
                return "%r11d";
            }
            return "%r11";
        }
        case REG_R12: {
            if (bytes <= 1) {
                return "%r12b";
            } else if (bytes <= 2) {
                return "%r12w";
            } else if (bytes <= 4) {
                return "%r12d";
            }
            return "%r12";
        }
        case REG_R13: {
            if (bytes <= 1) {
                return "%r13b";
            } else if (bytes <= 2) {
                return "%r13w";
            } else if (bytes <= 4) {
                return "%r13d";
            }
            return "%r13";
        }
        case REG_R14: {
            if (bytes <= 1) {
                return "%r14b";
            } else if (bytes <= 2) {
                return "%r14w";
            } else if (bytes <= 4) {
                return "%r14d";
            }
            return "%r14";
        }
        case REG_R15: {
            if (bytes <= 1) {
                return "%r15b";
            } else if (bytes <= 2) {
                return "%r15w";
            } else if (bytes <= 4) {
                return "%r15d";
            }
            return "%r15";
        }
    }
    assert(0 && "invalid register");
    return NULL;
}

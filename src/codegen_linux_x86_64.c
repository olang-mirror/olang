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

size_t label_index;

static void
codegen_linux_x86_64_emit_start_entrypoint(FILE *out);

static void
codegen_linux_x86_64_emit_function(FILE *out, ast_fn_definition_t *fn);

void
codegen_linux_x86_64_emit_program(FILE *out, ast_node_t *node)
{
    label_index = 0;
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

static size_t
codegen_linux_x86_64_get_next_label(void)
{
    return ++label_index;
}

static void
codegen_linux_x86_64_emit_expression(FILE *out, ast_node_t *expr_node)
{
    switch (expr_node->kind) {
        case AST_NODE_LITERAL: {
            ast_literal_t literal_u32 = expr_node->as_literal;
            assert(literal_u32.kind == AST_LITERAL_U32);
            uint32_t n = literal_u32.as_u32;

            fprintf(out, "    mov $%d, %%rax\n", n);
            return;
        }
        case AST_NODE_BINARY_OP: {
            ast_binary_op_t bin_op = expr_node->as_bin_op;
            switch (bin_op.kind) {
                case AST_BINOP_ADDITION: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    add %%rcx, %%rax\n");

                    return;
                }
                case AST_BINOP_MULTIPLICATION: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    mul %%rcx\n");

                    return;
                }
                case AST_BINOP_DIVISION: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    xor %%rdx, %%rdx\n");
                    fprintf(out, "    div %%rcx\n");

                    return;
                }
                case AST_BINOP_REMINDER: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");
                    fprintf(out, "    xor %%edx, %%edx\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    xor %%rdx, %%rdx\n");
                    fprintf(out, "    div %%rcx\n");
                    fprintf(out, "    mov %%edx, %%eax\n");

                    return;
                }
                case AST_BINOP_SUBTRACTION: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    sub %%rcx, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_EQ: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    cmp %%rcx, %%rax\n");
                    fprintf(out, "    sete %%al\n");
                    fprintf(out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_LT: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    cmp %%rcx, %%rax\n");
                    fprintf(out, "    setl %%al\n");
                    fprintf(out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_GT: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    cmp %%rcx, %%rax\n");
                    fprintf(out, "    setg %%al\n");
                    fprintf(out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_NEQ: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    cmp %%rcx, %%rax\n");
                    fprintf(out, "    setne %%al\n");
                    fprintf(out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_LEQ: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    cmp %%rcx, %%rax\n");
                    fprintf(out, "    setle %%al\n");
                    fprintf(out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_GEQ: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    cmp %%rcx, %%rax\n");
                    fprintf(out, "    setge %%al\n");
                    fprintf(out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_BITWISE_LSHIFT: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    shl %%cl, %%rax\n");

                    return;
                }
                case AST_BINOP_BITWISE_RSHIFT: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    shr %%cl, %%rax\n");

                    return;
                }
                case AST_BINOP_BITWISE_XOR: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    xor %%ecx, %%eax\n");

                    return;
                }
                case AST_BINOP_BITWISE_AND: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    and %%ecx, %%eax\n");

                    return;
                }
                case AST_BINOP_BITWISE_OR: {
                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    pop %%rcx\n");
                    fprintf(out, "    or %%ecx, %%eax\n");

                    return;
                }
                case AST_BINOP_LOGICAL_AND: {
                    size_t label_exit = codegen_linux_x86_64_get_next_label();

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    cmp $0, %%rax\n");
                    fprintf(out, "    je .L%ld\n", label_exit);

                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    cmp $0, %%rax\n");
                    fprintf(out, "    je .L%ld\n", label_exit);
                    fprintf(out, "    mov $1, %%rax\n");
                    fprintf(out, ".L%ld:\n", label_exit);

                    return;
                }
                case AST_BINOP_LOGICAL_OR: {
                    size_t label_t = codegen_linux_x86_64_get_next_label();
                    size_t label_f = codegen_linux_x86_64_get_next_label();

                    codegen_linux_x86_64_emit_expression(out, bin_op.lhs);
                    fprintf(out, "    cmp $0, %%rax\n");
                    fprintf(out, "    jne .L%ld\n", label_t);

                    codegen_linux_x86_64_emit_expression(out, bin_op.rhs);
                    fprintf(out, "    cmp $0, %%rax\n");
                    fprintf(out, "    je .L%ld\n", label_f);

                    fprintf(out, ".L%ld:\n", label_t);
                    fprintf(out, "    mov $1, %%rax\n");
                    fprintf(out, ".L%ld:\n", label_f);

                    return;
                }
                default: {
                    assert(0 && "unsupported binary operation");
                    return;
                }
            }
        }
        default:
            assert(0 && "unsupported expression");
    }
}
static void
codegen_linux_x86_64_emit_block(FILE *out, ast_block_t *block)
{

    size_t nodes_len = list_size(block->nodes);

    for (size_t i = 0; i < nodes_len; ++i) {
        ast_node_t *node = list_get(block->nodes, i)->value;
        switch (node->kind) {
            case AST_NODE_RETURN_STMT: {
                ast_return_stmt_t return_stmt = node->as_return_stmt;

                ast_node_t *expr = return_stmt.data;

                codegen_linux_x86_64_emit_expression(out, expr);

                fprintf(out, "    ret\n");

                break;
            }
            case AST_NODE_IF_STMT: {
                ast_if_stmt_t if_stmt = node->as_if_stmt;

                ast_node_t *cond = if_stmt.cond;
                ast_node_t *then = if_stmt.then;
                ast_node_t *_else = if_stmt._else;

                size_t end_if_label = codegen_linux_x86_64_get_next_label();
                size_t end_else_label = codegen_linux_x86_64_get_next_label();

                codegen_linux_x86_64_emit_expression(out, cond);
                fprintf(out, "    cmp $1, %%rax\n");
                fprintf(out, "    jnz .L%ld\n", end_if_label);

                assert(then->kind == AST_NODE_BLOCK && "invalid if-then block");
                ast_block_t then_block = then->as_block;

                codegen_linux_x86_64_emit_block(out, &then_block);
                fprintf(out, "    jmp .L%ld\n", end_else_label);

                fprintf(out, ".L%ld:\n", end_if_label);

                if (_else != NULL) {
                    ast_block_t else_block = _else->as_block;
                    codegen_linux_x86_64_emit_block(out, &else_block);
                }

                fprintf(out, ".L%ld:\n", end_else_label);

                break;
            }
            default: {
                assert(0 && "unsupported block statement");
                break;
            }
        }
    }
}

static void
codegen_linux_x86_64_emit_function(FILE *out, ast_fn_definition_t *fn)
{
    ast_node_t *block_node = fn->block;
    fprintf(out, "" SV_FMT ":\n", SV_ARG(fn->identifier));

    assert(block_node->kind == AST_NODE_BLOCK);
    ast_block_t block = block_node->as_block;

    codegen_linux_x86_64_emit_block(out, &block);
}

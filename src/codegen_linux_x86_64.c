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
#define PTR_HEX_CSTR_SIZE (18 + 1)

// The call instruction pushes EIP into stack so the first 8 bytes from stack
// must be preserved else the ret instruction will jump to nowere.
#define X86_CALL_EIP_STACK_OFFSET (8)

// FIXME: move label_index to codegen_linux_x86_64_t structure
size_t label_index;

static void
codegen_linux_x86_64_emit_start_entrypoint(codegen_x86_64_t *codegen);

static void
codegen_linux_x86_64_emit_function(codegen_x86_64_t *codegen, ast_fn_definition_t *fn);

static size_t
type_to_bytes(type_t *type);

static char *
get_accumulator_reg_for(size_t bytes);

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
codegen_linux_x86_64_emit_program(codegen_x86_64_t *codegen, ast_node_t *node)
{
    label_index = 0;
    codegen_linux_x86_64_emit_start_entrypoint(codegen);

    assert(node->kind == AST_NODE_PROGRAM);
    ast_program_t program = node->as_program;

    ast_fn_definition_t fn = program.fn->as_fn_def;

    assert(string_view_eq_to_cstr(fn.identifier, "main"));
    codegen_linux_x86_64_emit_function(codegen, &fn);
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
codegen_linux_x86_64_get_next_label(void)
{
    return ++label_index;
}

static void
codegen_linux_x86_64_emit_expression(codegen_x86_64_t *codegen, ast_node_t *expr_node)
{
    switch (expr_node->kind) {
        case AST_NODE_LITERAL: {
            ast_literal_t literal_u32 = expr_node->as_literal;
            assert(literal_u32.kind == AST_LITERAL_U32);
            uint32_t n = literal_u32.as_u32;

            fprintf(codegen->out, "    mov $%d, %%rax\n", n);
            return;
        }
        case AST_NODE_REF: {
            ast_ref_t ref = expr_node->as_ref;

            symbol_t *symbol = scope_lookup(ref.scope, ref.identifier);
            assert(symbol);

            char symbol_ptr[PTR_HEX_CSTR_SIZE];
            sprintf(symbol_ptr, "%p", (void *)symbol);

            size_t *offset = (size_t *)map_get(codegen->symbols_stack_offset, symbol_ptr);
            assert(offset);

            fprintf(codegen->out,
                    "    mov -%ld(%%rbp), %s\n",
                    *offset,
                    get_accumulator_reg_for(type_to_bytes(&symbol->type)));
            return;
        }
        case AST_NODE_BINARY_OP: {
            ast_binary_op_t bin_op = expr_node->as_bin_op;
            switch (bin_op.kind) {
                case AST_BINOP_ADDITION: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    add %%rcx, %%rax\n");

                    return;
                }
                case AST_BINOP_MULTIPLICATION: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    mul %%rcx\n");

                    return;
                }
                case AST_BINOP_DIVISION: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    xor %%rdx, %%rdx\n");
                    fprintf(codegen->out, "    div %%rcx\n");

                    return;
                }
                case AST_BINOP_REMINDER: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");
                    fprintf(codegen->out, "    xor %%edx, %%edx\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    xor %%rdx, %%rdx\n");
                    fprintf(codegen->out, "    div %%rcx\n");
                    fprintf(codegen->out, "    mov %%edx, %%eax\n");

                    return;
                }
                case AST_BINOP_SUBTRACTION: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    sub %%rcx, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_EQ: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    cmp %%rcx, %%rax\n");
                    fprintf(codegen->out, "    sete %%al\n");
                    fprintf(codegen->out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_LT: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    cmp %%rcx, %%rax\n");
                    fprintf(codegen->out, "    setl %%al\n");
                    fprintf(codegen->out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_GT: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    cmp %%rcx, %%rax\n");
                    fprintf(codegen->out, "    setg %%al\n");
                    fprintf(codegen->out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_NEQ: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    cmp %%rcx, %%rax\n");
                    fprintf(codegen->out, "    setne %%al\n");
                    fprintf(codegen->out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_LEQ: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    cmp %%rcx, %%rax\n");
                    fprintf(codegen->out, "    setle %%al\n");
                    fprintf(codegen->out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_CMP_GEQ: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    cmp %%rcx, %%rax\n");
                    fprintf(codegen->out, "    setge %%al\n");
                    fprintf(codegen->out, "    movzb %%al, %%rax\n");

                    return;
                }
                case AST_BINOP_BITWISE_LSHIFT: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    shl %%cl, %%rax\n");

                    return;
                }
                case AST_BINOP_BITWISE_RSHIFT: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    shr %%cl, %%rax\n");

                    return;
                }
                case AST_BINOP_BITWISE_XOR: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    xor %%ecx, %%eax\n");

                    return;
                }
                case AST_BINOP_BITWISE_AND: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    and %%ecx, %%eax\n");

                    return;
                }
                case AST_BINOP_BITWISE_OR: {
                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    push %%rax\n");

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    pop %%rcx\n");
                    fprintf(codegen->out, "    or %%ecx, %%eax\n");

                    return;
                }
                case AST_BINOP_LOGICAL_AND: {
                    size_t label_exit = codegen_linux_x86_64_get_next_label();

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    cmp $0, %%rax\n");
                    fprintf(codegen->out, "    je .L%ld\n", label_exit);

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    cmp $0, %%rax\n");
                    fprintf(codegen->out, "    je .L%ld\n", label_exit);
                    fprintf(codegen->out, "    mov $1, %%rax\n");
                    fprintf(codegen->out, ".L%ld:\n", label_exit);

                    return;
                }
                case AST_BINOP_LOGICAL_OR: {
                    size_t label_t = codegen_linux_x86_64_get_next_label();
                    size_t label_f = codegen_linux_x86_64_get_next_label();

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.lhs);
                    fprintf(codegen->out, "    cmp $0, %%rax\n");
                    fprintf(codegen->out, "    jne .L%ld\n", label_t);

                    codegen_linux_x86_64_emit_expression(codegen, bin_op.rhs);
                    fprintf(codegen->out, "    cmp $0, %%rax\n");
                    fprintf(codegen->out, "    je .L%ld\n", label_f);

                    fprintf(codegen->out, ".L%ld:\n", label_t);
                    fprintf(codegen->out, "    mov $1, %%rax\n");
                    fprintf(codegen->out, ".L%ld:\n", label_f);

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
                fprintf(codegen->out, "    ret\n");

                break;
            }

            case AST_NODE_VAR_DEF: {
                ast_var_definition_t var_def = node->as_var_def;
                scope_t *scope = var_def.scope;

                symbol_t *symbol = scope_lookup(scope, var_def.identifier);
                assert(symbol);

                char symbol_ptr[PTR_HEX_CSTR_SIZE];
                sprintf(symbol_ptr, "%p", (void *)symbol);

                if (var_def.value) {
                    codegen_linux_x86_64_emit_expression(codegen, var_def.value);
                }

                size_t *offset = arena_alloc(codegen->arena, sizeof(size_t));
                *offset = codegen->base_offset;

                map_put(codegen->symbols_stack_offset, symbol_ptr, offset);

                size_t type_size = type_to_bytes(&symbol->type);

                fprintf(codegen->out,
                        "    mov %s, -%ld(%%rbp)\n",
                        get_accumulator_reg_for(type_size),
                        codegen->base_offset);
                codegen->base_offset += type_size;

                break;
            }

            case AST_NODE_IF_STMT: {
                ast_if_stmt_t if_stmt = node->as_if_stmt;

                ast_node_t *cond = if_stmt.cond;
                ast_node_t *then = if_stmt.then;
                ast_node_t *_else = if_stmt._else;

                size_t end_if_label = codegen_linux_x86_64_get_next_label();
                size_t end_else_label = codegen_linux_x86_64_get_next_label();

                codegen_linux_x86_64_emit_expression(codegen, cond);
                fprintf(codegen->out, "    cmp $1, %%rax\n");
                fprintf(codegen->out, "    jnz .L%ld\n", end_if_label);

                assert(then->kind == AST_NODE_BLOCK && "invalid if-then block");
                ast_block_t then_block = then->as_block;

                codegen_linux_x86_64_emit_block(codegen, &then_block);
                fprintf(codegen->out, "    jmp .L%ld\n", end_else_label);

                fprintf(codegen->out, ".L%ld:\n", end_if_label);

                if (_else != NULL) {
                    ast_block_t else_block = _else->as_block;
                    codegen_linux_x86_64_emit_block(codegen, &else_block);
                }

                fprintf(codegen->out, ".L%ld:\n", end_else_label);

                break;
            }
            default: {
                // FIXME: improve error: replace the node->kind to a string representation
                fprintf(stderr, "node kind %d not supported\n", node->kind);
                assert(0 && "unsupported block statement");
                break;
            }
        }
    }

    codegen->base_offset = block_offset;
}

static size_t
type_to_bytes(type_t *type)
{
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            return type->as_primitive.size;
        }
    }

    assert(0 && "unreachable");
}

static size_t
calculate_fn_local_size(scope_t *scope)
{
    assert(scope);

    size_t local_size = 0;

    map_kv_t *kvs[scope->symbols->size];

    map_get_kvs(scope->symbols, kvs);

    for (size_t i = 0; i < scope->symbols->size; ++i) {
        symbol_t *symbol = (symbol_t *)kvs[i]->value;
        local_size += type_to_bytes(&symbol->type);
    }

    size_t max_child_local_size = 0;

    list_item_t *item = list_head(scope->children);

    while (item != NULL) {
        size_t child_local_size = calculate_fn_local_size((scope_t *)item->value);

        if (child_local_size > max_child_local_size) {
            max_child_local_size = child_local_size;
        }

        item = list_next(item);
    }

    return local_size + max_child_local_size;
}

static void
codegen_linux_x86_64_emit_function(codegen_x86_64_t *codegen, ast_fn_definition_t *fn)
{
    codegen->base_offset = X86_CALL_EIP_STACK_OFFSET;

    ast_node_t *block_node = fn->block;
    fprintf(codegen->out, "" SV_FMT ":\n", SV_ARG(fn->identifier));

    fprintf(codegen->out, "    mov %%rsp, %%rbp\n");

    size_t local_size = calculate_fn_local_size(fn->scope);

    if (local_size != 0) {
        fprintf(codegen->out, "    sub $%ld, %%rsp\n", local_size);
    }

    assert(block_node->kind == AST_NODE_BLOCK);
    ast_block_t block = block_node->as_block;

    codegen_linux_x86_64_emit_block(codegen, &block);
}

static char *
get_accumulator_reg_for(size_t bytes)
{
    if (bytes <= 1) {
        return "%ah";
    } else if (bytes <= 2) {
        return "%ax";
    } else if (bytes <= 4) {
        return "%eax";
    }
    return "%rax";
}

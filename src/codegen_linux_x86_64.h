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
#ifndef CODEGEN_X86_64_H
#define CODEGEN_X86_64_H

#include "arena.h"
#include "ast.h"
#include "map.h"
#include <stdio.h>

typedef struct codegen_x86_64
{
    arena_t *arena;
    size_t base_offset;
    size_t label_index;
    map_t *symbols_stack_offset;
    FILE *out;
} codegen_x86_64_t;

void
codegen_linux_x86_64_init(codegen_x86_64_t *codegen, arena_t *arena, FILE *out);

void
codegen_linux_x86_64_emit_program(codegen_x86_64_t *codegen, ast_node_t *prog);

#endif /* CODEGEN_X86_64_H */

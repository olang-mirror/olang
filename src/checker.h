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
#ifndef CHECKER_H
#define CHECKER_H

#include "arena.h"
#include "ast.h"

typedef struct checker
{
    arena_t *arena;
} checker_t;

checker_t *
checker_new(arena_t *arena);

void
checker_check(checker_t *checker, ast_node_t *ast);

#endif /* CHECKER_H */

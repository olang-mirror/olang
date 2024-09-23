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
#ifndef PARSER_H
#define PARSER_H

#include "arena.h"
#include "ast.h"
#include "lexer.h"

typedef struct parser
{
    lexer_t *lexer;
    arena_t *arena;
    // TODO: we should define a better place to file_path string
    char *file_path;
} parser_t;

void
parser_init(parser_t *parser, lexer_t *lexer, arena_t *arena, char *file_path);

ast_node_t *
parser_parse_translation_unit(parser_t *parser);

#endif /* PARSER_H */

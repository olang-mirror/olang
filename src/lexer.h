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
#ifndef LEXER_H
#define LEXER_H

#include "string_view.h"
#include <stdint.h>
#include <stdio.h>

typedef struct lexer
{
    string_view_t source;
    size_t offset;
    size_t row;
    size_t bol;
} lexer_t;

typedef enum token_kind
{
    TOKEN_UNKNOWN,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    // Keywords
    TOKEN_FN,
    TOKEN_RETURN,

    // Single char
    TOKEN_LF,
    TOKEN_OPAREN,
    TOKEN_CPAREN,
    TOKEN_COLON,
    TOKEN_OCURLY,
    TOKEN_CCURLY,
    TOKEN_EOF
} token_kind_t;

typedef struct token_loc
{
    size_t offset;
    size_t row;
    size_t bol;
} token_loc_t;

typedef struct token
{
    token_kind_t kind;
    string_view_t value;
    token_loc_t location;
} token_t;

void
lexer_init(lexer_t *lexer, string_view_t source);

void
lexer_next_token(lexer_t *lexer, token_t *token);

void
lexer_peek_next(lexer_t *lexer, token_t *token);

void
lexer_lookahead(lexer_t *lexer, token_t *token, size_t n);

char *
token_kind_to_cstr(token_kind_t kind);

string_view_t
lexer_get_token_line(lexer_t *lexer, token_t *token);

#endif /* LEXER_H */

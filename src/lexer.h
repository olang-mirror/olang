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

typedef struct source_code
{
    char *filepath;
    string_view_t code;
} source_code_t;

typedef struct lexer_cursor
{
    size_t offset;
    size_t row;
    size_t bol;
} lexer_cursor_t;

typedef struct lexer
{
    source_code_t src;
    lexer_cursor_t cur;
} lexer_t;

typedef enum token_kind
{
    TOKEN_UNKNOWN,
    TOKEN_ID,
    TOKEN_NUMBER,

    // Keywords
    TOKEN_FN,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_VAR,

    // Equality operators
    TOKEN_CMP_EQ,
    TOKEN_CMP_NEQ,
    TOKEN_CMP_LEQ,
    TOKEN_CMP_GEQ,

    // Logical Operators
    TOKEN_LOGICAL_OR,
    TOKEN_LOGICAL_AND,

    // Bitwise Operators
    TOKEN_BITWISE_LSHIFT,
    TOKEN_BITWISE_RSHIFT,

    // Single char
    TOKEN_BANG,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_PERCENT,
    TOKEN_AND,
    TOKEN_PIPE,
    TOKEN_CIRCUMFLEX,
    TOKEN_EQ,
    TOKEN_PLUS,
    TOKEN_DASH,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_LF,
    TOKEN_OPAREN,
    TOKEN_CPAREN,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_OCURLY,
    TOKEN_CCURLY,
    TOKEN_EOF
} token_kind_t;

typedef struct token_loc
{
    source_code_t src;
    lexer_cursor_t cur;
} token_loc_t;

typedef struct token
{
    token_kind_t kind;
    string_view_t value;
    token_loc_t loc;
} token_t;

size_t
token_loc_to_lineno(token_loc_t loc);

size_t
token_loc_to_colno(token_loc_t loc);

void
lexer_init(lexer_t *lexer, source_code_t src);

void
lexer_next_token(lexer_t *lexer, token_t *token);

void
lexer_peek_next(lexer_t *lexer, token_t *token);

void
lexer_lookahead(lexer_t *lexer, token_t *token, size_t n);

char *
token_kind_to_cstr(token_kind_t kind);

bool
token_kind_is_binary_op(token_kind_t kind);

string_view_t
token_loc_to_line(token_loc_t loc);

#endif /* LEXER_H */

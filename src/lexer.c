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
#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>

void
lexer_init(lexer_t *lexer, string_view_t source)
{
    assert(lexer);
    lexer->source = source;
    lexer->offset = 0;
    lexer->row = 0;
    lexer->bol = 0;
}

static char
lexer_current_char(lexer_t *lexer);

static void
lexer_skip_char(lexer_t *lexer);

static bool
lexer_is_eof(lexer_t *lexer);

static bool
lexer_is_not_eof(lexer_t *lexer);

static bool
_isspace(char c);

static void
lexer_init_char_value_token(lexer_t *lexer, token_t *token, token_kind_t kind);

static void
lexer_init_str_value_token(lexer_t *lexer, token_t *token, token_kind_t kind, size_t start_offset);

static void
lexer_init_eof_token(lexer_t *lexer, token_t *token);

static token_kind_t
lexer_str_to_token_kind(string_view_t text);

void
lexer_next_token(lexer_t *lexer, token_t *token)
{
    if (lexer_is_eof(lexer)) {
        lexer_init_eof_token(lexer, token);
        return;
    }

    char current_char = lexer_current_char(lexer);

    if (_isspace(current_char)) {
        while (_isspace(current_char) && lexer_is_not_eof(lexer)) {
            lexer_skip_char(lexer);
            current_char = lexer_current_char(lexer);
        }
    }

    while (lexer_is_not_eof(lexer)) {
        if (isalpha(current_char)) {
            size_t start_offset = lexer->offset;
            while (isalnum(current_char) && lexer_is_not_eof(lexer)) {
                lexer_skip_char(lexer);
                current_char = lexer_current_char(lexer);
            }

            string_view_t text = { .chars = lexer->source.chars + start_offset, .size = lexer->offset - start_offset };

            lexer_init_str_value_token(lexer, token, lexer_str_to_token_kind(text), start_offset);
            return;
        }

        if (isdigit(current_char)) {
            size_t start_offset = lexer->offset;
            while (isdigit(current_char) && lexer_is_not_eof(lexer)) {
                lexer_skip_char(lexer);
                current_char = lexer_current_char(lexer);
            }

            lexer_init_str_value_token(lexer, token, TOKEN_NUMBER, start_offset);
            return;
        }

        switch (current_char) {
            case '(': {
                lexer_init_char_value_token(lexer, token, TOKEN_OPAREN);
                lexer_skip_char(lexer);
                return;
            }
            case ')': {
                lexer_init_char_value_token(lexer, token, TOKEN_CPAREN);
                lexer_skip_char(lexer);
                return;
            }
            case ':': {
                lexer_init_char_value_token(lexer, token, TOKEN_COLON);
                lexer_skip_char(lexer);
                return;
            }
            case '{': {
                lexer_init_char_value_token(lexer, token, TOKEN_OCURLY);
                lexer_skip_char(lexer);
                return;
            }
            case '}': {
                lexer_init_char_value_token(lexer, token, TOKEN_CCURLY);
                lexer_skip_char(lexer);
                return;
            }
            case '\n': {
                lexer_init_char_value_token(lexer, token, TOKEN_LF);
                lexer_skip_char(lexer);
                return;
            }
            default: {
                lexer_init_char_value_token(lexer, token, TOKEN_UNKNOWN);
                lexer_skip_char(lexer);
                return;
            }
        }
    }

    if (lexer_is_eof(lexer)) {
        lexer_init_eof_token(lexer, token);
        return;
    }
}

static char *token_kind_str_table[] = {
    [TOKEN_UNKNOWN] = "unknown", [TOKEN_IDENTIFIER] = "identifier",
    [TOKEN_NUMBER] = "number",   [TOKEN_FN] = "fn",
    [TOKEN_RETURN] = "return",   [TOKEN_LF] = "line_feed",
    [TOKEN_OPAREN] = "(",        [TOKEN_CPAREN] = ")",
    [TOKEN_COLON] = ":",         [TOKEN_OCURLY] = "{",
    [TOKEN_CCURLY] = "}",        [TOKEN_EOF] = "EOF",
};

char *
token_kind_to_cstr(token_kind_t kind)
{
    assert(kind < sizeof(token_kind_str_table));
    return token_kind_str_table[kind];
}

static char
lexer_current_char(lexer_t *lexer)
{
    return lexer->source.chars[lexer->offset];
}

static void
lexer_skip_char(lexer_t *lexer)
{
    assert(lexer->offset < lexer->source.size);
    if (lexer_current_char(lexer) == '\n') {
        lexer->row++;
        lexer->bol = ++lexer->offset;
    } else {
        lexer->offset++;
    }
}

static bool
lexer_is_eof(lexer_t *lexer)
{
    return lexer->offset >= lexer->source.size;
}

static bool
lexer_is_not_eof(lexer_t *lexer)
{
    return !lexer_is_eof(lexer);
}

static bool
_isspace(char c)
{
    return c != '\n' && isspace(c);
}

static void
lexer_init_char_value_token(lexer_t *lexer, token_t *token, token_kind_t kind)
{
    string_view_t str = { .chars = lexer->source.chars + lexer->offset, .size = 1 };
    token_loc_t location = { .offset = lexer->offset, .row = lexer->row, .bol = lexer->bol };
    *token = (token_t){ .kind = kind, .value = str, .location = location };
}

static void
lexer_init_str_value_token(lexer_t *lexer, token_t *token, token_kind_t kind, size_t start_offset)
{
    string_view_t str = { .chars = lexer->source.chars + start_offset, .size = lexer->offset - start_offset };
    token_loc_t location = { .offset = start_offset, .row = lexer->row, .bol = lexer->bol };
    *token = (token_t){ .kind = kind, .value = str, .location = location };
}

static void
lexer_init_eof_token(lexer_t *lexer, token_t *token)
{
    string_view_t str = { 0 };
    token_loc_t location = { .offset = lexer->offset, .row = lexer->row, .bol = lexer->bol };
    *token = (token_t){ .kind = TOKEN_EOF, .value = str, .location = location };
}

static token_kind_t
lexer_str_to_token_kind(string_view_t text)
{
    if (string_view_eq_to_cstr(text, "return")) {
        return TOKEN_RETURN;
    }

    if (string_view_eq_to_cstr(text, "fn")) {
        return TOKEN_FN;
    }

    return TOKEN_IDENTIFIER;
}

void
lexer_peek_next(lexer_t *lexer, token_t *token)
{
    lexer_lookahead(lexer, token, 1);
}

void
lexer_lookahead(lexer_t *lexer, token_t *token, size_t n)
{
    size_t previous_offset = lexer->offset;
    size_t previous_row = lexer->row;
    size_t previous_bol = lexer->bol;

    for (size_t i = 0; i < n; ++i) {
        lexer_next_token(lexer, token);
    }

    lexer->offset = previous_offset;
    lexer->row = previous_row;
    lexer->bol = previous_bol;
}

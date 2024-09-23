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
#include <stdio.h>

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
        if (current_char == '#') {
            while (current_char != '\n' && lexer_is_not_eof(lexer)) {
                lexer_skip_char(lexer);
                current_char = lexer_current_char(lexer);
            }
        }

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
            case '=': {
                size_t start_offset = lexer->offset;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '=') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_CMP_EQ, start_offset);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_EQ, start_offset);
                return;
            }
            case '!': {
                size_t start_offset = lexer->offset;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '=') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_CMP_NEQ, start_offset);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_BANG, start_offset);
                return;
            }
            case '&': {
                size_t start_offset = lexer->offset;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '&') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_LOGICAL_AND, start_offset);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_AND, start_offset);
                return;
            }
            case '|': {
                size_t start_offset = lexer->offset;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '|') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_LOGICAL_OR, start_offset);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_PIPE, start_offset);
                return;
            }
            case '<': {
                size_t start_offset = lexer->offset;
                lexer_skip_char(lexer);

                switch (lexer_current_char(lexer)) {
                    case '<': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_BITWISE_LSHIFT, start_offset);
                        return;
                    }
                    case '=': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_CMP_LEQ, start_offset);
                        return;
                    }
                    default: {
                        lexer_init_str_value_token(lexer, token, TOKEN_LT, start_offset);
                        return;
                    }
                }
            }
            case '>': {
                size_t start_offset = lexer->offset;
                lexer_skip_char(lexer);

                switch (lexer_current_char(lexer)) {
                    case '>': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_BITWISE_RSHIFT, start_offset);
                        return;
                    }
                    case '=': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_CMP_GEQ, start_offset);
                        return;
                    }
                    default: {
                        lexer_init_str_value_token(lexer, token, TOKEN_GT, start_offset);
                        return;
                    }
                }
            }
            case '^': {
                lexer_init_char_value_token(lexer, token, TOKEN_CIRCUMFLEX);
                lexer_skip_char(lexer);
                return;
            }
            case '%': {
                lexer_init_char_value_token(lexer, token, TOKEN_PERCENT);
                lexer_skip_char(lexer);
                return;
            }
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
            case '+': {
                lexer_init_char_value_token(lexer, token, TOKEN_PLUS);
                lexer_skip_char(lexer);
                return;
            }
            case '-': {
                lexer_init_char_value_token(lexer, token, TOKEN_DASH);
                lexer_skip_char(lexer);
                return;
            }
            case '*': {
                lexer_init_char_value_token(lexer, token, TOKEN_STAR);
                lexer_skip_char(lexer);
                return;
            }
            case '/': {
                lexer_init_char_value_token(lexer, token, TOKEN_SLASH);
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
    [TOKEN_UNKNOWN] = "unknown",
    [TOKEN_ID] = "identifier",
    [TOKEN_NUMBER] = "number",
    [TOKEN_FN] = "fn",
    [TOKEN_RETURN] = "return",
    [TOKEN_IF] = "if",
    [TOKEN_ELSE] = "else",
    [TOKEN_VAR] = "var",
    [TOKEN_LF] = "line_feed",
    [TOKEN_OPAREN] = "(",
    [TOKEN_CPAREN] = ")",
    [TOKEN_COLON] = ":",
    [TOKEN_OCURLY] = "{",
    [TOKEN_CCURLY] = "}",
    [TOKEN_PLUS] = "+",
    [TOKEN_DASH] = "-",
    [TOKEN_STAR] = "*",
    [TOKEN_SLASH] = "/",
    [TOKEN_EQ] = "=",
    [TOKEN_CMP_EQ] = "==",
    [TOKEN_BANG] = "!",
    [TOKEN_CMP_NEQ] = "!=",
    [TOKEN_LT] = "<",
    [TOKEN_GT] = ">",
    [TOKEN_CMP_LEQ] = "<=",
    [TOKEN_CMP_GEQ] = ">=",
    [TOKEN_PERCENT] = "%",
    [TOKEN_BITWISE_LSHIFT] = "<<",
    [TOKEN_BITWISE_RSHIFT] = ">>",
    [TOKEN_CIRCUMFLEX] = "^",
    [TOKEN_PIPE] = "|",
    [TOKEN_LOGICAL_OR] = "||",
    [TOKEN_AND] = "&",
    [TOKEN_LOGICAL_AND] = "&&",
    [TOKEN_EOF] = "EOF",
};

char *
token_kind_to_cstr(token_kind_t kind)
{
    assert(kind < sizeof(token_kind_str_table));
    return token_kind_str_table[kind];
}

bool
token_kind_is_binary_op(token_kind_t kind)
{
    switch (kind) {
        case TOKEN_PLUS:
        case TOKEN_DASH:
        case TOKEN_SLASH:
        case TOKEN_STAR:
        case TOKEN_PERCENT:
        case TOKEN_BITWISE_LSHIFT:
        case TOKEN_BITWISE_RSHIFT:
        case TOKEN_LT:
        case TOKEN_CMP_LEQ:
        case TOKEN_GT:
        case TOKEN_CMP_GEQ:
        case TOKEN_CMP_EQ:
        case TOKEN_CMP_NEQ:
        case TOKEN_AND:
        case TOKEN_CIRCUMFLEX:
        case TOKEN_PIPE:
        case TOKEN_LOGICAL_AND:
        case TOKEN_LOGICAL_OR:
            return true;
        default:
            return false;
    }
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
    if (string_view_eq_to_cstr(text, "if")) {
        return TOKEN_IF;
    }

    if (string_view_eq_to_cstr(text, "else")) {
        return TOKEN_ELSE;
    }

    if (string_view_eq_to_cstr(text, "var")) {
        return TOKEN_VAR;
    }

    if (string_view_eq_to_cstr(text, "return")) {
        return TOKEN_RETURN;
    }

    if (string_view_eq_to_cstr(text, "fn")) {
        return TOKEN_FN;
    }

    return TOKEN_ID;
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

string_view_t
lexer_get_token_line(lexer_t *lexer, token_t *token)
{
    size_t offset = token->location.bol;
    string_view_t line = { .chars = lexer->source.chars + offset, .size = 0 };

    while ((line.size + offset) < lexer->source.size && line.chars[line.size] != '\n' && line.chars[line.size] != 0) {
        ++line.size;
    }

    return line;
}

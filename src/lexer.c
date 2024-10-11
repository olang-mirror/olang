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
lexer_init(lexer_t *lexer, source_code_t src)
{
    assert(lexer);
    lexer->src = src;
    lexer->cur.offset = 0;
    lexer->cur.row = 0;
    lexer->cur.bol = 0;
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
lexer_init_str_value_token(lexer_t *lexer, token_t *token, token_kind_t kind, lexer_cursor_t cur);

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
            lexer_cursor_t start_cur = lexer->cur;
            while (isalnum(current_char) && lexer_is_not_eof(lexer)) {
                lexer_skip_char(lexer);
                current_char = lexer_current_char(lexer);
            }

            string_view_t text = {
                .chars = lexer->src.code.chars + start_cur.offset,
                .size = lexer->cur.offset - start_cur.offset,
            };

            lexer_init_str_value_token(lexer, token, lexer_str_to_token_kind(text), start_cur);
            return;
        }

        if (isdigit(current_char)) {
            lexer_cursor_t start_cur = lexer->cur;
            while (isdigit(current_char) && lexer_is_not_eof(lexer)) {
                lexer_skip_char(lexer);
                current_char = lexer_current_char(lexer);
            }

            lexer_init_str_value_token(lexer, token, TOKEN_NUMBER, start_cur);
            return;
        }

        switch (current_char) {
            case '=': {
                lexer_cursor_t start_cur = lexer->cur;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '=') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_CMP_EQ, start_cur);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_EQ, start_cur);
                return;
            }
            case '!': {
                lexer_cursor_t start_cur = lexer->cur;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '=') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_CMP_NEQ, start_cur);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_BANG, start_cur);
                return;
            }
            case '&': {
                lexer_cursor_t start_cur = lexer->cur;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '&') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_LOGICAL_AND, start_cur);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_AND, start_cur);
                return;
            }
            case '|': {
                lexer_cursor_t start_cur = lexer->cur;
                lexer_skip_char(lexer);

                if (lexer_current_char(lexer) == '|') {
                    lexer_skip_char(lexer);
                    lexer_init_str_value_token(lexer, token, TOKEN_LOGICAL_OR, start_cur);
                    return;
                }

                lexer_init_str_value_token(lexer, token, TOKEN_PIPE, start_cur);
                return;
            }
            case '<': {
                lexer_cursor_t start_cur = lexer->cur;
                lexer_skip_char(lexer);

                switch (lexer_current_char(lexer)) {
                    case '<': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_BITWISE_LSHIFT, start_cur);
                        return;
                    }
                    case '=': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_CMP_LEQ, start_cur);
                        return;
                    }
                    default: {
                        lexer_init_str_value_token(lexer, token, TOKEN_LT, start_cur);
                        return;
                    }
                }
            }
            case '>': {
                lexer_cursor_t start_cur = lexer->cur;
                lexer_skip_char(lexer);

                switch (lexer_current_char(lexer)) {
                    case '>': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_BITWISE_RSHIFT, start_cur);
                        return;
                    }
                    case '=': {
                        lexer_skip_char(lexer);
                        lexer_init_str_value_token(lexer, token, TOKEN_CMP_GEQ, start_cur);
                        return;
                    }
                    default: {
                        lexer_init_str_value_token(lexer, token, TOKEN_GT, start_cur);
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
            case ',': {
                lexer_init_char_value_token(lexer, token, TOKEN_COMMA);
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
            case '~': {
                lexer_init_char_value_token(lexer, token, TOKEN_TILDE);
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
    [TOKEN_WHILE] = "while",
    [TOKEN_VAR] = "var",
    [TOKEN_LF] = "line_feed",
    [TOKEN_OPAREN] = "(",
    [TOKEN_CPAREN] = ")",
    [TOKEN_COLON] = ":",
    [TOKEN_COMMA] = ",",
    [TOKEN_OCURLY] = "{",
    [TOKEN_CCURLY] = "}",
    [TOKEN_PLUS] = "+",
    [TOKEN_DASH] = "-",
    [TOKEN_STAR] = "*",
    [TOKEN_SLASH] = "/",
    [TOKEN_TILDE] = "~",
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
        case TOKEN_EQ:
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
    return lexer->src.code.chars[lexer->cur.offset];
}

static void
lexer_skip_char(lexer_t *lexer)
{
    assert(lexer->cur.offset < lexer->src.code.size);
    if (lexer_current_char(lexer) == '\n') {
        lexer->cur.row++;
        lexer->cur.bol = ++lexer->cur.offset;
    } else {
        lexer->cur.offset++;
    }
}

static bool
lexer_is_eof(lexer_t *lexer)
{
    return lexer->cur.offset >= lexer->src.code.size;
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
    string_view_t str = {
        .chars = lexer->src.code.chars + lexer->cur.offset,
        .size = 1,
    };
    *token = (token_t){
        .kind = kind,
        .value = str,
        .loc =
            (token_loc_t){
                .src = lexer->src,
                .cur = lexer->cur,
            },
    };
}

static void
lexer_init_str_value_token(lexer_t *lexer, token_t *token, token_kind_t kind, lexer_cursor_t cur)
{
    string_view_t str = {
        .chars = lexer->src.code.chars + cur.offset,
        .size = lexer->cur.offset - cur.offset,
    };
    *token = (token_t){
        .kind = kind,
        .value = str,
        .loc =
            (token_loc_t){
                .src = lexer->src,
                .cur = cur,
            },
    };
}

static void
lexer_init_eof_token(lexer_t *lexer, token_t *token)
{
    string_view_t str = { 0 };
    *token = (token_t){
        .kind = TOKEN_EOF,
        .value = str,
        .loc =
            (token_loc_t){
                .src = lexer->src,
                .cur = lexer->cur,
            },
    };
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

    if (string_view_eq_to_cstr(text, "while")) {
        return TOKEN_WHILE;
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
    lexer_cursor_t previous_cur = lexer->cur;

    for (size_t i = 0; i < n; ++i) {
        lexer_next_token(lexer, token);
    }

    lexer->cur = previous_cur;
}

string_view_t
token_loc_to_line(token_loc_t loc)
{
    size_t offset = loc.cur.bol;
    string_view_t line = {
        .chars = loc.src.code.chars + offset,
        .size = 0,
    };

    while ((line.size + offset) < loc.src.code.size && line.chars[line.size] != '\n' && line.chars[line.size] != 0) {
        ++line.size;
    }

    return line;
}

size_t
token_loc_to_lineno(token_loc_t loc)
{
    return loc.cur.row + 1;
}

size_t
token_loc_to_colno(token_loc_t loc)
{
    return loc.cur.offset - loc.cur.bol + 1;
}

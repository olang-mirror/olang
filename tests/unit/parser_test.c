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
#define MUNIT_ENABLE_ASSERT_ALIASES

#include "arena.h"
#include "ast.h"
#include "lexer.h"
#include "list.h"
#include "munit.h"
#include "parser.h"
#include "string_view.h"

#define ARENA_CAPACITY (1024 * 1024)

static MunitResult
parse_translation_unit_test(const MunitParameter params[], void *user_data_or_fixture)
{
    arena_t arena = arena_new(ARENA_CAPACITY);

    char *filepath = "main.0";
    char *source_value = "fn main(): u32 {\n\treturn 69\n}";

    lexer_t lexer;
    string_view_t code = { .chars = source_value, .size = strlen(source_value) };
    lexer_init(&lexer, (source_code_t){ .code = code, .filepath = filepath });

    parser_t parser;
    parser_init(&parser, &lexer, &arena, filepath);

    ast_node_t *translation_unit_node = parser_parse_translation_unit(&parser);
    assert_not_null(translation_unit_node);
    assert_uint(translation_unit_node->kind, ==, AST_NODE_TRANSLATION_UNIT);

    ast_translation_unit_t translation_unit = translation_unit_node->as_translation_unit;

    assert_uint(list_size(translation_unit.decls), ==, 1);

    ast_node_t *fn_node = (ast_node_t *)list_head(translation_unit.decls)->value;

    assert_not_null(fn_node);
    assert_uint(fn_node->kind, ==, AST_NODE_FN_DEF);

    ast_fn_definition_t fn = fn_node->as_fn_def;
    assert_memory_equal(fn.id.size, fn.id.chars, "main");
    assert_memory_equal(fn.return_type.size, fn.return_type.chars, "u32");

    ast_node_t *block = fn.block;
    assert_not_null(block);

    assert_uint(block->kind, ==, AST_NODE_BLOCK);
    assert_uint(list_size(block->as_block.nodes), ==, 1);
    list_item_t *block_item = list_get(block->as_block.nodes, 0);
    assert_not_null(block_item);
    assert_not_null(block_item->value);

    ast_node_t *node = (ast_node_t *)block_item->value;
    assert_not_null(node);
    assert_uint(node->kind, ==, AST_NODE_RETURN_STMT);

    ast_node_t *number_node = node->as_return_stmt.expr;
    assert_not_null(number_node);
    assert_uint(number_node->kind, ==, AST_NODE_LITERAL);
    assert_uint(number_node->as_literal.kind, ==, AST_LITERAL_U32);
    assert_uint(number_node->as_literal.as_u32, ==, 69);

    arena_free(&arena);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    { "/parse_translation_unit", parse_translation_unit_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = { "/parser", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
    return EXIT_SUCCESS;
}

# Copyright (C) 2024 olang mantainers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

fn main(argc: u8, argv: u64): u8 {
  return 0
}

# TEST test_contains_tokens WITH
# ./0027_function_with_params.ol:16:1: <fn>
# ./0027_function_with_params.ol:16:4: <identifier>
# ./0027_function_with_params.ol:16:8: <(>
# ./0027_function_with_params.ol:16:9: <identifier>
# ./0027_function_with_params.ol:16:13: <:>
# ./0027_function_with_params.ol:16:15: <identifier>
# ./0027_function_with_params.ol:16:17: <,>
# ./0027_function_with_params.ol:16:19: <identifier>
# ./0027_function_with_params.ol:16:23: <:>
# ./0027_function_with_params.ol:16:25: <identifier>
# ./0027_function_with_params.ol:16:28: <)>
# ./0027_function_with_params.ol:16:29: <:>
# ./0027_function_with_params.ol:16:31: <identifier>
# ./0027_function_with_params.ol:16:34: <{>
# ./0027_function_with_params.ol:16:35: <line_feed>
# ./0027_function_with_params.ol:17:3: <return>
# ./0027_function_with_params.ol:17:10: <number>
# ./0027_function_with_params.ol:17:11: <line_feed>
# ./0027_function_with_params.ol:18:1: <}>
# END

# TEST test_ast WITH
# Translation_Unit
# `-Function_Definition <name:main> <return:u8>
#   |-Param_Definition <name:argc> <type:u8>
#   |-Param_Definition <name:argv> <type:u64>
#   `-Block
#     `-Return_Statement
#       `-Literal <kind:u32> <value:0>
# END

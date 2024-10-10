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

fn main(): u32 {
  var i: u32 = 0

  while i < 10 {
    i = i + 1
  }

  return i
}

# TEST test_compile(exit_code=0)
#
# TEST test_run_binary(exit_code=10)
#
# TEST test_ast WITH
# Translation_Unit
# `-Function_Definition <name:main> <return:u32>
#   `-Block
#     |-Var_Definition <name:i> <kind:u32>
#     | `-Literal <kind:u32> <value:0>
#     |-While_Statement
#     | |-Binary_Operation (<)
#     | | |-Reference <name:i>
#     | | `-Literal <kind:u32> <value:10>
#     | `-Block
#     |   `-Binary_Operation (=)
#     |     |-Reference <name:i>
#     |     `-Binary_Operation (+)
#     |       |-Reference <name:i>
#     |       `-Literal <kind:u32> <value:1>
#     `-Return_Statement
#       `-Reference <name:i>
# END
#
# TEST test_contains_tokens WITH
# ./0030_while_statement.ol:19:3: <while>
# END

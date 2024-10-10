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
  var a: u32 = 1
  var b: u32* = &a
  *b = 0
  return a
}

# xTEST test_compile(exit_code=0)
#
# xTEST test_run_binary(exit_code=0)
#
# TEST test_ast WITH
# Translation_Unit
# `-Function_Definition <name:main> <return:u32>
#   `-Block
#     |-Var_Definition <name:a> <kind:u32>
#     | `-Literal <kind:u32> <value:1>
#     |-Var_Definition <name:b> <kind:u32*>
#     | `-Unary_Operation (&)
#     |   `-Reference <name:a>
#     |-Binary_Operation (=)
#     | |-Unary_Operation (*)
#     | | `-Reference <name:b>
#     | `-Literal <kind:u32> <value:0>
#     `-Return_Statement
#       `-Reference <name:a>
# END

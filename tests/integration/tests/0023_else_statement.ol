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
  if 0 != 0 {
    return 1
  } else {
    return 0
  }
}

# TEST test_compile(exit_code=0)

# TEST test_run_binary(exit_code=0)

# TEST test_contains_tokens WITH
# ./tests/0023_else_statement.ol:19:5: <else>
# END

# TEST test_ast WITH
# Translation_Unit
# `-Function_Definition <name:main> <return:0>
#   `-Block
#     `-If_Statement
#       |-Binary_Operation (!=)
#       | |-Literal <kind:u32> <value:0>
#       | `-Literal <kind:u32> <value:0>
#       |-Block
#       | `-Return_Statement
#       |   `-Literal <kind:u32> <value:1>
#       `-Block
#         `-Return_Statement
#           `-Literal <kind:u32> <value:0>
# END

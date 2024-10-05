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

fn main(): u8 {
  return add(40, 2) 
}

fn add(a: u32, b: u64): u8 {
  return a + b
}

# TEST test_compile(exit_code=0)

# TEST test_run_binary(exit_code=42)

# TEST test_ast WITH
# Translation_Unit
# |-Function_Definition <name:main> <return:u8>
# | `-Block
# |   `-Return_Statement
# |     `-Function_Call <name:add>
# |       |-Literal <kind:u32> <value:40>
# |       `-Literal <kind:u32> <value:2>
# `-Function_Definition <name:add> <return:u8>
#   |-Param_Definition <name:a> <type:u32>
#   |-Param_Definition <name:b> <type:u64>
#   `-Block
#     `-Return_Statement
#       `-Binary_Operation (+)
#         |-Reference <name:a>
#         `-Reference <name:b>
# END

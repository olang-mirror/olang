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

# XTEST test_compile(exit_code=0)
#
# XTEST test_run_binary(exit_code=9)
#
# XTEST test_ast WITH
# Translation_Unit
# `-Function_Definition <name:main> <return:u32>
#   `-Block
#     `-Return_Statement
#       `-Literal <kind:u32> <value:0>
# END
#
# TEST test_contains_tokens WITH
# ./0030_while_statement.ol:19:3: <while>
# END

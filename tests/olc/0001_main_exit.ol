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

# A minimal olang program
fn main(): u32 {
  return 0
}

# TEST test_compile(exit_code=0)
#
# TEST test_run_binary(exit_code=0)
#
# TEST test_ast WITH
# Translation_Unit
# `-Function_Definition <name:main> <return:u32>
#   `-Block
#     `-Return_Statement
#       `-Literal <kind:u32> <value:0>
# END
#
# TEST test_contains_tokens WITH
# ./0001_main_exit.ol:17:1: <fn>
# ./0001_main_exit.ol:17:4: <identifier>
# ./0001_main_exit.ol:17:8: <(>
# ./0001_main_exit.ol:17:9: <)>
# ./0001_main_exit.ol:17:10: <:>
# ./0001_main_exit.ol:17:12: <identifier>
# ./0001_main_exit.ol:17:16: <{>
# ./0001_main_exit.ol:17:17: <line_feed>
# ./0001_main_exit.ol:18:3: <return>
# ./0001_main_exit.ol:18:10: <number>
# ./0001_main_exit.ol:18:11: <line_feed>
# ./0001_main_exit.ol:19:1: <}>
# END

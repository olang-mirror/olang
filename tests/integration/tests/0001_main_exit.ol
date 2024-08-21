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
# `-Function_Definition <name:main> <return:0>
#   `-Block
#     `-Return_Statement
#       `-Literal <kind:u32> <value:0>
# END
#
# TEST test_contains_tokens WITH
# ./tests/0001_main_exit.ol:17:1: <fn>
# ./tests/0001_main_exit.ol:17:4: <identifier>
# ./tests/0001_main_exit.ol:17:8: <(>
# ./tests/0001_main_exit.ol:17:9: <)>
# ./tests/0001_main_exit.ol:17:10: <:>
# ./tests/0001_main_exit.ol:17:12: <identifier>
# ./tests/0001_main_exit.ol:17:16: <{>
# ./tests/0001_main_exit.ol:17:17: <line_feed>
# ./tests/0001_main_exit.ol:18:3: <return>
# ./tests/0001_main_exit.ol:18:10: <number>
# ./tests/0001_main_exit.ol:18:11: <line_feed>
# ./tests/0001_main_exit.ol:19:1: <}>
# END

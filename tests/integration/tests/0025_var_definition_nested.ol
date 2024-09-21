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

  if a == 1 {
    var b: u32 = 43
    var c: u32 = 2

    return a + b - c
  } else {
    var b: u32 = 41

    return a + b
  }

  return a
}

# TEST test_compile(exit_code=0)

# TEST test_run_binary(exit_code=42)

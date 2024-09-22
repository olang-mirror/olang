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

fn main(): u64 {
  var a: u8 = 255
  var b: u16 = 65535
  var c: u32 = 4294967295
  var d: u64 = 4294967296

  return a + b + c + d - a - b - c - d
}

# TEST test_compile(exit_code=0)

# TEST test_run_binary(exit_code=0)

#!/bin/sh
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

# Ignores variables been modified in a subshell
# shellcheck disable=SC2030,SC2031
OLANG_PATH="../../olc"
TEST_FILE="$1"

TEST_TMP_FILES="$TEST_FILE.test"
TEST_TMP_BIN="$TEST_TMP_FILES.bin"

# Execution state vars
TEST_NAME=""
TEST_INVOCATION=""
TEST_LINE_NUMBER=""
TEST_CONTENTS_PATH=""
TEST_ARGS=""
TEST_SOME_PASSED=""

# UI
COLOR_RED=1
COLOR_GREEN=2
COLOR_YELLOW=3
COLOR_CYAN=6
COLOR_GRAY=7

colored() {
  text="$1"

  if [ -t 1 ]; then
    if tput setaf 1 > /dev/null 2>&1; then
      color=$(tput setaf "$2")
      reset=$(tput sgr0)

      printf "%s%s%s" "$color" "$text" "$reset"
      return
    fi
  fi

  printf "%s" "$text"
}
# end UI

# test output
print_failed() {
  reason="$1"

  if [ -n "$TEST_SOME_PASSED" ]; then
    echo
  fi

  colored "$TEST_FILE:$TEST_LINE_NUMBER:1: " $COLOR_GRAY
  colored "[$TEST_INVOCATION] " $COLOR_CYAN
  colored "failed" $COLOR_RED
  if [ -n "$reason" ]; then
    printf ": %s" "$reason"
  fi
  echo
}

print_passed() {
  colored "." $COLOR_GREEN
  TEST_SOME_PASSED="true"
}
# end test output

# expectations
expect_output_contains() {
  actual_file="$1"
  expected_file="$2"
  original_line_number="$TEST_LINE_NUMBER"

  while read -r expected_line ; do
    TEST_LINE_NUMBER=$((TEST_LINE_NUMBER+1))

    if [ "$(grep "$(printf "%s" "$expected_line" | sed 's/\\/\\\\/g')" "$actual_file" | wc -c)" = "0" ]; then
      print_failed
      colored "(not found) $expected_line" $COLOR_YELLOW
      echo
      colored "$(awk '{print "(actual) " $0}' "$actual_file")" $COLOR_GRAY
      echo
      exit 1
    fi
  done < "$expected_file"

  TEST_LINE_NUMBER="$original_line_number"
}

diff_output() {
  actual_file="$1"
  expected_file="$2"

  if cmp -s "$expected_file" "$actual_file"; then
    return
  fi

  print_failed "match failed"
  diff "$actual_file" "$expected_file" -u --color
  exit 1
}

cleanup() {
  rm -rf "$TEST_TMP_FILES"*
}

main() {
  all_test_end="$(grep -n "# END" "$TEST_FILE" | awk -F: '{print $1}')"

  grep -n "# TEST " "$TEST_FILE" | while read -r line ; do
    TEST_LINE_NUMBER="$(echo "$line" | awk -F: '{ print $1 }')"
    TEST_INVOCATION="$(echo "$line" | awk -F: '{ print $2 }' | sed 's/^\# TEST //' | sed 's/ WITH$//')"

    TEST_NAME="$(echo "$TEST_INVOCATION" | awk -F\( '{ print $1 }')"
    TEST_ARGS="$(echo "$TEST_INVOCATION" | awk -F\( '{ print $2 }' | sed 's/)$//')"

    has_with_contents="$(echo "$line" | sed 's/.*WITH$/true/')"
    end_line="$TEST_LINE_NUMBER"

    if [ "$has_with_contents" = "true" ]; then
      for test_end in $all_test_end; do
        if [ "$test_end" -gt "$TEST_LINE_NUMBER" ]; then
          end_line="$test_end"
          break
        fi
      done

      TEST_CONTENTS_PATH="$TEST_TMP_FILES.$TEST_LINE_NUMBER.$end_line.partial"

      awk -v start="$TEST_LINE_NUMBER" -v end="$end_line" 'NR > start && NR < end' "$TEST_FILE" \
        | sed 's/^\# /#/' | sed 's/^\#//' > "$TEST_CONTENTS_PATH"
    else
      TEST_CONTENTS_PATH=""
    fi

    if type "$TEST_NAME" | grep -q 'function'; then
        "$TEST_NAME"
        print_passed
    else
        print_failed "test not implemented"
    fi
  done || exit 1;

  cleanup
  echo
}

get_test_args() {
  arg_name="$1"
  echo "$TEST_ARGS" | sed "s/,/\n/g" | grep "$arg_name=" | sed "s/^$arg_name=//"
}

assert_contents_path() {
  if [ -z "$TEST_CONTENTS_PATH" ]; then
    print_failed "missing WITH block for test contents"
    exit 5
  fi
}

# Test functions
test_compile() {
  expected_exit_code="$(get_test_args "exit_code")"
  actual_output_file="$TEST_TMP_FILES.$TEST_LINE_NUMBER.compiler_output"

  $OLANG_PATH "$TEST_FILE" -o "$TEST_TMP_BIN" > "$actual_output_file" 2>&1
  exit_code="$?"

  if [ -n "$expected_exit_code" ]; then
    if [ "$expected_exit_code" -ne "$exit_code" ]; then
      print_failed "expected compiler exit code: $expected_exit_code actual: $exit_code"
      exit 1
    fi
  fi

  if [ -n "$TEST_CONTENTS_PATH" ]; then
    diff_output "$actual_output_file" "$TEST_CONTENTS_PATH"
  fi
}

test_ast() {
  assert_contents_path

  actual_output_file="$TEST_TMP_FILES.$TEST_LINE_NUMBER.ast_output"

  $OLANG_PATH "$TEST_FILE" --dump-ast > "$actual_output_file" 2>&1

  diff_output "$actual_output_file" "$TEST_CONTENTS_PATH"
}

test_contains_tokens() {
  assert_contents_path

  actual_output_file="$TEST_TMP_FILES.$TEST_LINE_NUMBER.tokens_output"

  $OLANG_PATH "$TEST_FILE" --dump-tokens > "$actual_output_file" 2>&1

  expect_output_contains "$actual_output_file" "$TEST_CONTENTS_PATH"
}

test_run_binary() {
  expected_exit_code="$(get_test_args "exit_code")"
  actual_output_file="$TEST_TMP_FILES.$TEST_LINE_NUMBER.run_output"

  "$TEST_TMP_BIN" > "$actual_output_file" 2>&1
  exit_code="$?"

  if [ -n "$expected_exit_code" ]; then
    if [ "$expected_exit_code" -ne "$exit_code" ]; then
      print_failed "expected compiler exit code: $expected_exit_code actual: $exit_code"
      exit 1
    fi
  fi

  if [ -n "$TEST_CONTENTS_PATH" ]; then
    diff_output "$actual_output_file" "$TEST_CONTENTS_PATH"
  fi
}

main

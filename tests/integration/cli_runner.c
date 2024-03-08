/*
 * Copyright (C) 2024 olang maintainers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "cli_runner.h"
#include "proc_exec.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define OLANG_COMPILER_PATH "../../0c"

static int compiler_exists_already_checked = 0;

static void
assert_compiler_exists()
{
    {
        if (compiler_exists_already_checked == 1) {
            return;
        }

        compiler_exists_already_checked = 1;
    }

    FILE *file = fopen(OLANG_COMPILER_PATH, "r");

    if (file != NULL) {
        fclose(file);
        return;
    }

    perror("Build the compiler before executing tests");
    exit(1);
}

void
create_tmp_file_name(char *file_name)
{
    sprintf(file_name, "%s/olang_programXXXXXX", P_tmpdir);
    int fd = mkstemp(file_name);

    if (fd == -1) {
        perror("Could not create a tmp file. Check your P_tmpdir permission.");
        exit(1);
    }
    close(fd);
}

void
cli_runner_compiler(cli_result_t* result, char *args[])
{
    assert_compiler_exists();

    proc_exec_command_t command = {
        .path = OLANG_COMPILER_PATH,
        .args = args
    };

    proc_exec(&command);

    result->exec = command.result;
}

cli_result_t
cli_runner_compiler_dump_tokens(char *src)
{
    cli_result_t result = { 0 };

    char *program_args[] = { "0c", "--dump-tokens", src, NULL };
    cli_runner_compiler(&result, program_args);
    return result;
}

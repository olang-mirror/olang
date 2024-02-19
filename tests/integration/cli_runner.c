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

cli_result_t
cli_runner_compiler(char *src, char *args[])
{
    assert_compiler_exists();

    cli_result_t result = { 0 };
    create_tmp_file_name(result.program_path);

    int fd_link[2];

    if (pipe(fd_link) == -1) {
        perror("pipe error.");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork error.");
        exit(1);
    }

    if (pid == 0) {
        dup2(fd_link[1], STDOUT_FILENO);
        close(fd_link[0]);
        close(fd_link[1]);

        execv(OLANG_COMPILER_PATH, args);
        perror("execl error.");
        exit(127);
    } else {
        close(fd_link[1]);
        if (read(fd_link[0], result.compiler_output, sizeof(result.compiler_output)) == -1) {
            perror("read error.");
            exit(1);
        }
        int status;
        waitpid(pid, &status, 0);
        result.exit_code = WEXITSTATUS(status);
    }

    return result;
}

cli_result_t
cli_runner_compiler_dump_tokens(char *src)
{
    char *program_args[] = { "0c", "--dump-tokens", src, NULL };
    return cli_runner_compiler(src, program_args);
}

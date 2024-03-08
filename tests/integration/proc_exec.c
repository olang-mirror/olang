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
#include "proc_exec.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


void
proc_exec(proc_exec_command_t* command)
{
    int fd_pipe[2];

    if (pipe(fd_pipe) == -1) {
        perror("pipe error.");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork error.");
        exit(1);
    }

    if (pid == 0) {
        dup2(fd_pipe[1], STDOUT_FILENO);
        close(fd_pipe[0]);
        close(fd_pipe[1]);

        execv(command->path, command->args);
        perror("execl error.");
        exit(127);
    } else {
        close(fd_pipe[1]);
        // TODO: stop truncating the output.
        if (read(fd_pipe[0], command->result.stdout_buf, sizeof(command->result.stdout_buf)) == -1) {
            perror("read error.");
            exit(1);
        }
        int status;
        waitpid(pid, &status, 0);
        command->result.exit_code = WEXITSTATUS(status);
    }
}

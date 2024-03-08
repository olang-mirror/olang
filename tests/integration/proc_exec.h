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
#ifndef PROC_EXEC_H
#define PROC_EXEC_H
#include <stdlib.h>

typedef struct proc_exec_result
{
    int exit_code;
    char stdout_buf[1024];
} proc_exec_result_t;

typedef struct proc_exec_command
{
    char* path;
    char** args;
    proc_exec_result_t result;
} proc_exec_command_t;

void
proc_exec(proc_exec_command_t* command);
#endif

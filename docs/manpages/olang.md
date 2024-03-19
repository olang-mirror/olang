% OLANG(1)
% olang mantainers
% Feb 2024

# NAME

olang - O Programming Language compiler

# SYNOPSIS

**olang**
    source_file
    [**----dump-tokens**]
    [**--o** ___output_file___ [**----save-temps**] [**----arch** ___arch___] [**----sysroot** ___dir___]]

# DESCRIPTION

**olang** is the offical O programming language compiler, it is also a tool that
contains utilities to help the language development.

# GENERAL OPTIONS

**----dump-tokens**
:   Display lexical tokens given a soruce.0 code.

**--o** ___file___
:   Compile program into a binary file

**----save-temps**
:   Keep temp files used to compile program

**----arch** ___architecture___
:   Binary arch: default to "x86_64", avaliable options ("x86_64" | "aarch64")

**----sysroot**  ___dir___
:   System root dir where the GNU Assembler and GNU Linker are located: default to '/'

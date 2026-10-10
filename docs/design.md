# ShellForge - System Design

## 1. Project Overview
ShellForge is a Unix-like command shell implemented in C.

## 2. Architecture
Input → Parser → Built-in/External → Process Management → Pipes → Redirection → Signals → Threads → Job Control

## 3. Modules
- Input: reads commands
- Parser: tokenizes input
- Process: fork/exec/waitpid
- Pipes: IPC using pipe()
- Redirection: stdin/stdout/stderr redirection
- Signals: SIGINT, SIGTSTP, SIGCHLD
- Threads: POSIX thread monitor
- Jobs: background job table
- Job Control: process groups, fg, bg

## 4. Build
make clean && make

## 5. Run
make run

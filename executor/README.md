This folder contains my solution to one of the assignments in Concurrent Programming class - written in C.

Assignment: create a program "executor" which can simultaneously run other tasks and store their latest stderr & stdout output.

Executor has following commands:

1. "run A B C ..." starts task running program A with arguments B C ... in the background. Prints "Task T started: pid P.\n", where T is task number and P is pid of the ran program.
2. "out T" prints "Task T stdout: 'S'.\n" where S is last line written to stdout by task T.
3. "err T" prints "Task T stderr: 'S'.\n" where S is last line written to stderr by task T.
4. "kill T" sends SIGINT to program A in task T.
5. "sleep N" makes executor sleep for N milliseconds.
6. "quit" ends executor and all programs.

Assumptions/constraints:

1. When program ends, it waits until executor finishes current task and
   then prints "Task T ended: status X.\n" where T is task id and X is exit code. When program is signalled it instead prints "Task T ended: signalled.\n".
2. Executor memory is independent from output length and duration of executed programs.
3. Executed programs have only standard descriptors open.
4. It is assumed that executor commands are correct and at most 511 characters long.
5. It is assumed that output from executed programs will be at most 1022 characters long.
6. It is assumed that there will be at most 4096 tasks. (4096 "run" commands).
7. It is assumed that programs will never output null character and executor commands will never contain null, quote or backslash characters.

Executor prints a lot of diagnostic information to stderr.

err._ and utils._ files were provided.

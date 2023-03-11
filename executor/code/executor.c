#include "err.h"
#include "task.h"
#include "utils.h"
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_N_TASKS 4096
#define INPUT_SIZE 512

int main()
{
    // +++++ Declarations & setup +++++
    // Magic spells  //CORRECTION: commented out magic spells
    // ASSERT_ZERO(setvbuf(stdout, NULL, _IOLBF, 1024));
    // ASSERT_ZERO(setvbuf(stderr, NULL, _IOLBF, 1024));
    // Tasks shared memory setup
    Task* tasks = (Task*)malloc(MAX_N_TASKS * sizeof(Task));
    char* raw_input = (char*)malloc(INPUT_SIZE * sizeof(char));

    sem_t* output_mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (tasks == NULL || raw_input == NULL || output_mutex == MAP_FAILED) {
        exit(1);
    }
    ASSERT_SYS_OK(sem_init(output_mutex, 1, 1));

    char** input;
    int n = 0; // next task to run

    // +++++ Listening for commands +++++
    while (read_line(raw_input, INPUT_SIZE, stdin)) {
        input = split_string(raw_input);
        ASSERT_SYS_OK(sem_wait(output_mutex));
        fprintf(stderr, "Received command: ");
        for (size_t i = 0; input[i] != NULL; i++)
            fprintf(stderr, "\"%s\" ", input[i]);
        fprintf(stderr, "\n");

        if (!strcmp(input[0], "run")) { // RUN
            tasks[n] = initialise_task_memory(n);

            // Creating the Manager process, who will wait for Task and Relays
            pid_t man_pid;
            ASSERT_SYS_OK(man_pid = fork());
            if (!man_pid) { // MANAGER

                // Creating two pipes for output. (0 - read, 1 - write)
                int out_pipe[2];
                int err_pipe[2];
                ASSERT_SYS_OK(pipe(out_pipe));
                ASSERT_SYS_OK(pipe(err_pipe));

                // CORRECTION:
                int fake_pipe[2];
                ASSERT_SYS_OK(pipe(fake_pipe));
                ASSERT_SYS_OK(dup2(fake_pipe[0], STDIN_FILENO));
                ASSERT_SYS_OK(close(fake_pipe[0]));
                ASSERT_SYS_OK(close(fake_pipe[1]));

                // Creating the task process
                pid_t task_pid;
                ASSERT_SYS_OK(task_pid = fork());
                if (!task_pid) { // TASK
                    //  Directing out & err output to pipes.
                    ASSERT_SYS_OK(close(out_pipe[0]));
                    ASSERT_SYS_OK(dup2(out_pipe[1], STDOUT_FILENO));
                    ASSERT_SYS_OK(close(out_pipe[1]));

                    ASSERT_SYS_OK(close(err_pipe[0]));
                    ASSERT_SYS_OK(dup2(err_pipe[1], STDERR_FILENO));
                    ASSERT_SYS_OK(close(err_pipe[1]));

                    //   Executing the command.
                    ASSERT_SYS_OK(execvp(input[1], input + 1));
                    // fprintf(stderr, "Executing command ")
                }

                // Writing the pid of the Task process to shared memory, so Executor has access to it
                set_pid(tasks[n], task_pid);

                // Subsequent code will have no use for write ends of pipes
                ASSERT_SYS_OK(close(out_pipe[1]));
                ASSERT_SYS_OK(close(err_pipe[1]));

                // Creating the Relays: they redirect their input to proper pipe and wait for input.

                // Creating the Out Relay
                pid_t pid1;
                ASSERT_SYS_OK(pid1 = fork());
                if (!pid1) { // OUT_RELAY
                    // ASSERT_SYS_OK(dup2(out_pipe[0], STDIN_FILENO));
                    // ASSERT_SYS_OK(close(out_pipe[0]));
                    ASSERT_SYS_OK(close(err_pipe[0]));
                    return write_out(tasks[n], out_pipe[0]);
                }

                // Creating the Err Relay
                pid_t pid2;
                ASSERT_SYS_OK(pid2 = fork());
                if (!pid2) { // ERR_RELAY
                    // ASSERT_SYS_OK(dup2(err_pipe[0], STDIN_FILENO));
                    ASSERT_SYS_OK(close(out_pipe[0]));
                    // ASSERT_SYS_OK(close(err_pipe[0]));
                    return write_err(tasks[n], err_pipe[0]);
                }

                // Waiting for Task process to end.
                int wstatus;
                ASSERT_SYS_OK(waitpid(task_pid, &wstatus, 0));
                fprintf(stderr, "Manager %d: task finished, commencing cleanup.\n", n);
                // When Task finishes, killing and waiting for Relays
                ASSERT_SYS_OK(kill(pid1, SIGKILL));
                ASSERT_SYS_OK(kill(pid2, SIGKILL));
                ASSERT_SYS_OK(wait(NULL));
                ASSERT_SYS_OK(wait(NULL));
                fprintf(stderr, "Manager %d: relays finished.\n", n);

                // Waiting for the output_mutex to print proper message.
                ASSERT_SYS_OK(sem_wait(output_mutex));
                if (WIFEXITED(wstatus)) {
                    printf("Task %d ended: status %d.\n", n, WEXITSTATUS(wstatus));
                } else if (WIFSIGNALED(wstatus)) {
                    printf("Task %d ended: signalled.\n", n);
                }
                ASSERT_SYS_OK(sem_post(output_mutex));

                return 0;
            } // End of Manager code.
            printf("Task %d started: pid %d.\n", n, get_pid(tasks[n]));
            n++;
            // End of RUN branch.
        } else if (!strcmp(input[0], "out")) { // OUT
            size_t i = atoi(input[1]);
            print_out(tasks[i], i);
        } else if (!strcmp(input[0], "err")) { // ERR
            size_t i = atoi(input[1]);
            print_err(tasks[i], i);
        } else if (!strcmp(input[0], "kill")) { // KILL
            size_t i = atoi(input[1]);
            kill(get_pid(tasks[i]), SIGINT);
        } else if (!strcmp(input[0], "sleep")) { // SLEEP
            int i = atoi(input[1]);
            // CORRECTION: prints to stderr instead of stdout
            fprintf(stderr, "Sleeping for %d miliseconds\n", i);
            ASSERT_SYS_OK(usleep(i * 1000));
        } else if (!strcmp(input[0], "quit")) { // QUIT
            free_split_string(input);
            fprintf(stderr, "Quiting.\n");
            ASSERT_SYS_OK(sem_post(output_mutex));

            break;
        }
        ASSERT_SYS_OK(sem_post(output_mutex));
        free_split_string(input);
    }

    // +++++ Cleanup +++++
    for (size_t i = 0; i < n; i++) {
        kill(get_pid(tasks[i]), SIGKILL);
    }
    fprintf(stderr, "All SIGKILL signals sent, waiting for managers.\n");
    // fprintf(stderr, "Managers waiting: %zu\n", *managers_waiting);

    for (size_t i = 0; i < n; i++) {
        ASSERT_SYS_OK(wait(NULL));
    }
    for (size_t i = 0; i < n; i++) {
        free_task_memory(tasks[i]);
    }
    free(raw_input);
    free(tasks);
    ASSERT_SYS_OK(sem_destroy(output_mutex));

    return 0;
}

#include "task.h"
#include "buffer.h"
#include "err.h"
#include "utils.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

struct task {
    pid_t pid;
    sem_t trig_pid;
    bool first;
    Buffer out;
    Buffer err;
};

Task initialise_task_memory(int name)
{
    Task task = mmap(NULL, sizeof(struct task), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (task == MAP_FAILED)
        syserr("mmap");

    char tname[9];
    sprintf(tname, "%d out", name);
    task->out = initialise_buffer(tname);
    sprintf(tname, "%d err", name);
    task->err = initialise_buffer(tname);

    ASSERT_SYS_OK(sem_init(&task->trig_pid, 1, 0));
    task->first = true;
    return task;
}

void free_task_memory(Task task)
{
    free_buffer(task->out);
    free_buffer(task->err);
    ASSERT_SYS_OK(sem_destroy(&task->trig_pid));

    munmap(task, sizeof(struct task));
}

void set_pid(Task task, pid_t pid)
{
    task->pid = pid;
    ASSERT_SYS_OK(sem_post(&task->trig_pid));
}

pid_t get_pid(Task task)
{
    if (task->first) {
        ASSERT_SYS_OK(sem_wait(&task->trig_pid));
        task->first = false;
    }
    return task->pid;
}

static bool read_and_check(FILE* stream, char* buffer)
{
    if (!fgets(buffer, OUTPUT_SIZE - 1, stream))
        return false;

    size_t read_len = strlen(buffer);
    if (buffer[read_len - 1] == '\n')
        buffer[read_len - 1] = '\0';
    return true;
}

int write_out(Task task, int desc)
{
    FILE* stream = fdopen(desc, "r");
    if (!stream) {
        exit(1);
    }
    char* buffer = (char*)calloc(OUTPUT_SIZE, sizeof(char));
    while (read_and_check(stream, buffer)) {
        write_to(task->out, buffer);
    }
    free(buffer);
    return 0;
}

int write_err(Task task, int desc)
{
    FILE* stream = fdopen(desc, "r");
    if (!stream) {
        exit(1);
    }
    char* buffer = (char*)calloc(OUTPUT_SIZE, sizeof(char));
    while (read_and_check(stream, buffer)) {
        write_to(task->err, buffer);
    }
    free(buffer);
    return 0;
}

void print_out(Task task, size_t n)
{
    char* help = read_from(task->out);
    printf("Task %zu stdout: '%s'.\n", n, help);
    free(help);
}

void print_err(Task task, size_t n)
{
    char* help = read_from(task->err);
    printf("Task %zu stderr: '%s'.\n", n, help);
    free(help);
}

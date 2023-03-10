#ifndef TASK_H
#define TASK_H

#include <semaphore.h>
#include <sys/types.h>

#define OUTPUT_SIZE 1024

typedef struct task* Task;

/* typedef struct task {
    pid_t pid;
    sem_t trig_pid;
    sem_t out_mutex;
    sem_t err_mutex;
    char* out;
    char* err;
} * Task; */

extern Task initialise_task_memory(int name);
extern void free_task_memory(Task task);

extern void set_pid(Task task, pid_t pid);
extern pid_t get_pid(Task task);

extern int write_out(Task task, int desc);
extern int write_err(Task task, int desc);

extern void print_out(Task task, size_t n);
extern void print_err(Task task, size_t n);

#endif // TASK_H

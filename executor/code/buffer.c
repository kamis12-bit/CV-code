#include "buffer.h"
#include "err.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define OUTPUT_SIZE 1024

struct buffer {
    sem_t mutex;
    char* space;
    char name[9];
};

Buffer initialise_buffer(const char* name)
{
    Buffer buffer = mmap(NULL, sizeof(struct buffer), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED) {
        syserr("mmap");
    }
    memcpy(buffer->name, name, 8);
    buffer->name[8] = '\0';
    buffer->space = mmap(NULL, OUTPUT_SIZE * sizeof(char), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (buffer->space == MAP_FAILED) {
        syserr("mmap");
    }
    ASSERT_SYS_OK(sem_init(&buffer->mutex, 1, 1));

    return buffer;
}

void free_buffer(Buffer buffer)
{
    ASSERT_SYS_OK(sem_destroy(&buffer->mutex));
    munmap(buffer->space, OUTPUT_SIZE * sizeof(char));
    munmap(buffer, sizeof(struct buffer));
}

void write_to(Buffer buffer, const char* message)
{
    ASSERT_SYS_OK(sem_wait(&buffer->mutex));
    memcpy(buffer->space, message, OUTPUT_SIZE - 1);
    ASSERT_SYS_OK(sem_post(&buffer->mutex));
    fprintf(stderr, "%s: written \"%s\"\n", buffer->name, buffer->space);
}

char* read_from(Buffer buffer)
{
    char* temp = (char*)calloc(OUTPUT_SIZE, sizeof(char));
    if (temp == NULL) {
        syserr("calloc");
    }
    ASSERT_SYS_OK(sem_wait(&buffer->mutex));
    memcpy(temp, buffer->space, OUTPUT_SIZE - 1);
    ASSERT_SYS_OK(sem_post(&buffer->mutex));
    fprintf(stderr, "%s: read \"%s\"\n", buffer->name, temp);
    return temp;
}

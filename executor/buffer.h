#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer* Buffer;

extern Buffer initialise_buffer(const char* name);
extern void free_buffer(Buffer buffer);

extern void write_to(Buffer buffer, const char* message);
extern char* read_from(Buffer buffer);

#endif // BUFFER_H

// include/ipc.h
#ifndef IPC_H
#define IPC_H

#include <stddef.h>

typedef enum {
    IPC_MMAP_SHARED,
    IPC_MMAP_PRIVATE
} mmap_mode_t;

typedef enum {
    IPC_SHM_POSIX,
} shm_type_t;

typedef enum {
    IPC_PIPE,
    IPC_FIFO
} pipe_type_t;

typedef struct {
    size_t message_size;
    size_t message_count;
    mmap_mode_t mmap_mode;
    shm_type_t shm_type;
    pipe_type_t pipe_type;
} ipc_params_t;

typedef void (*ipc_method_t)(const ipc_params_t* params);

void ipc_mmap(const ipc_params_t* params);
void ipc_shared_memory(const ipc_params_t* params);
void ipc_pipe(const ipc_params_t* params);
void ipc_fifo(const ipc_params_t* params);
void ipc_file(const ipc_params_t* params);
void ipc_message_queue(const ipc_params_t* params);
void ipc_unix_socket(const ipc_params_t* params);

void ipc_measure_capacity_message_queue();
void ipc_measure_capacity_pipe();
void ipc_measure_capacity_unix_socket();

#endif // IPC_H

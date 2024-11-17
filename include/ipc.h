// include/ipc.h
#ifndef IPC_H
#define IPC_H

#include <stddef.h>

typedef enum {
    IPC_MMAP_SHARED,
    IPC_MMAP_PRIVATE
} mmap_mode_t;

typedef struct {
    size_t message_size;
    size_t message_count;
    mmap_mode_t mmap_mode; // Нове поле для режиму mmap
} ipc_params_t;

typedef void (*ipc_method_t)(const ipc_params_t* params);

void ipc_mmap(const ipc_params_t* params);

// Інші методи будуть додані пізніше

#endif // IPC_H

#ifndef IPC_MMAP_H
#define IPC_MMAP_H

#include <stddef.h>

int ipc_mmap_init(int mode, size_t size);
int ipc_mmap_send(const void *message, size_t size);
int ipc_mmap_receive(void *buffer, size_t size);
size_t ipc_mmap_get_capacity();
void ipc_mmap_cleanup();

#endif // IPC_MMAP_H

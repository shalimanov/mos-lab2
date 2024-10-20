#ifndef IPC_SHM_H
#define IPC_SHM_H

#include <stddef.h>

int ipc_shm_init(size_t size);
int ipc_shm_send(const void *message, size_t size);
int ipc_shm_receive(void *buffer, size_t size);
size_t ipc_shm_get_capacity();
void ipc_shm_cleanup();

#endif // IPC_SHM_H

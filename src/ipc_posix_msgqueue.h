#ifndef IPC_POSIX_MSGQUEUE_H
#define IPC_POSIX_MSGQUEUE_H

#include <stddef.h>

int ipc_posix_msgqueue_init();
int ipc_posix_msgqueue_send(const void *message, size_t size);
int ipc_posix_msgqueue_receive(void *buffer, size_t size);
size_t ipc_posix_msgqueue_get_capacity();
void ipc_posix_msgqueue_cleanup();

#endif // IPC_POSIX_MSGQUEUE_H

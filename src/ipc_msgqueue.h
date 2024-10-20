#ifndef IPC_MSGQUEUE_H
#define IPC_MSGQUEUE_H

#include <stddef.h>

int ipc_msgqueue_init();
int ipc_msgqueue_send(const void *message, size_t size);
int ipc_msgqueue_receive(void *buffer, size_t size);
size_t ipc_msgqueue_get_capacity();
void ipc_msgqueue_cleanup();

#endif // IPC_MSGQUEUE_H

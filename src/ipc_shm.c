#include "ipc_shm.h"
#include "utils.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

static int shm_id = -1;
static void *shm_addr = NULL;
static size_t shm_size = 0;

int ipc_shm_init(size_t size) {
    key_t key = ftok("shmfile", 65); // Генерація ключа
    shm_id = shmget(key, size, 0666 | IPC_CREAT);
    if (shm_id == -1) {
        log_error("Не вдалося створити сегмент спільної пам'яті");
        return -1;
    }

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1) {
        log_error("Не вдалося підключити сегмент спільної пам'яті");
        return -1;
    }

    shm_size = size;
    return 0;
}

int ipc_shm_send(const void *message, size_t size) {
    if (size > shm_size) {
        log_error("Розмір повідомлення перевищує розмір сегмента спільної пам'яті");
        return -1;
    }
    memcpy(shm_addr, message, size);
    return 0;
}

int ipc_shm_receive(void *buffer, size_t size) {
    if (size > shm_size) {
        log_error("Розмір буфера перевищує розмір сегмента спільної пам'яті");
        return -1;
    }
    memcpy(buffer, shm_addr, size);
    return 0;
}

size_t ipc_shm_get_capacity() {
    return shm_size;
}

void ipc_shm_cleanup() {
    if (shm_addr != NULL) {
        shmdt(shm_addr);
        shm_addr = NULL;
    }
    if (shm_id != -1) {
        shmctl(shm_id, IPC_RMID, NULL);
        shm_id = -1;
    }
}

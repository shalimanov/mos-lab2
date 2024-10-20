#include "ipc_mmap.h"
#include "utils.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static int fd = -1;
static void *map = NULL;
static size_t map_size = 0;

int ipc_mmap_init(int mode, size_t size) {
    fd = shm_open("ipc_mmap", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        log_error("Не вдалося відкрити спільну пам'ять");
        return -1;
    }

    if (ftruncate(fd, size) == -1) {
        log_error("Не вдалося встановити розмір спільної пам'яті");
        close(fd);
        shm_unlink("ipc_mmap");
        return -1;
    }

    map = mmap(NULL, size, PROT_READ | PROT_WRITE, mode, fd, 0);
    if (map == MAP_FAILED) {
        log_error("Не вдалося відобразити спільну пам'ять");
        close(fd);
        shm_unlink("ipc_mmap");
        return -1;
    }

    map_size = size;
    return 0;
}

int ipc_mmap_send(const void *message, size_t size) {
    if (size > map_size) {
        log_error("Розмір повідомлення перевищує розмір відображення");
        return -1;
    }
    memcpy(map, message, size);
    return 0;
}

int ipc_mmap_receive(void *buffer, size_t size) {
    if (size > map_size) {
        log_error("Розмір буфера перевищує розмір відображення");
        return -1;
    }
    memcpy(buffer, map, size);
    return 0;
}

size_t ipc_mmap_get_capacity() {
    return map_size;
}

void ipc_mmap_cleanup() {
    if (map != NULL) {
        munmap(map, map_size);
        map = NULL;
    }
    if (fd != -1) {
        close(fd);
        shm_unlink("ipc_mmap");
        fd = -1;
    }
}

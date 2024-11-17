// src/ipc_mmap.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void ipc_mmap(const ipc_params_t* params) {
    const char* shm_name = "/ipc_mmap_shm";
    size_t total_size = params->message_size * params->message_count;
    int fd;
    void* addr;
    int mmap_flags;

    if (params->mmap_mode == IPC_MMAP_SHARED) {
        fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
        if (fd == -1) {
            perror("shm_open");
            exit(EXIT_FAILURE);
        }

        if (ftruncate(fd, total_size) == -1) {
            perror("ftruncate");
            shm_unlink(shm_name);
            close(fd);
            exit(EXIT_FAILURE);
        }

        mmap_flags = MAP_SHARED;
    } else { // IPC_MMAP_PRIVATE
        fd = open("/dev/zero", O_RDWR);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        mmap_flags = MAP_PRIVATE;
    }

    addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, mmap_flags, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        if (params->mmap_mode == IPC_MMAP_SHARED) {
            shm_unlink(shm_name);
        }
        close(fd);
        exit(EXIT_FAILURE);
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Імітація запису повідомлень
    for (size_t i = 0; i < params->message_count; ++i) {
        memset((char*)addr + i * params->message_size, 'A', params->message_size);
    }

    // Імітація читання повідомлень
    for (size_t i = 0; i < params->message_count; ++i) {
        char buffer[params->message_size];
        memcpy(buffer, (char*)addr + i * params->message_size, params->message_size);
        // Обробка повідомлення при необхідності
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_sec = end.tv_sec - start.tv_sec;
    double elapsed_nsec = end.tv_nsec - start.tv_nsec;
    double elapsed = elapsed_sec + elapsed_nsec / 1e9;

    printf("Elapsed time: %f seconds\n", elapsed);

    // Розрахунок пропускної здатності
    double throughput = (double)(params->message_size * params->message_count) / elapsed;
    printf("Throughput: %f bytes/second\n", throughput);

    munmap(addr, total_size);
    if (params->mmap_mode == IPC_MMAP_SHARED) {
        shm_unlink(shm_name);
    }
    close(fd);
}

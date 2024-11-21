#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

void ipc_shared_memory(const ipc_params_t* params) {
    const char* shm_name = "/ipc_shm";
    size_t total_size = params->message_size * params->message_count + 2 * sizeof(int);

    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
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

    void* addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        shm_unlink(shm_name);
        close(fd);
        exit(EXIT_FAILURE);
    }

    int* write_flag = (int*)addr;
    int* read_flag = (int*)addr + 1;
    void* data_addr = (char*)addr + 2 * sizeof(int);

    *write_flag = 0;
    *read_flag = 1;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        munmap(addr, total_size);
        shm_unlink(shm_name);
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Reader
        char* buffer = malloc(params->message_size);
        if (!buffer) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < params->message_count; ++i) {
            // Wait for the writer to signal data is ready
            while (*write_flag == 0) {
                usleep(1); // Prevent CPU spinning
            }

            memcpy(buffer, (char*)data_addr + i * params->message_size, params->message_size);

            // Signal to the writer that data has been consumed
            *write_flag = 0;
            __sync_synchronize();
            *read_flag = 1;
        }

        free(buffer);
        munmap(addr, total_size);
        close(fd);
        exit(EXIT_SUCCESS);
    } else { // Writer
        char* message = malloc(params->message_size);
        if (!message) {
            perror("malloc");
            munmap(addr, total_size);
            shm_unlink(shm_name);
            close(fd);
            exit(EXIT_FAILURE);
        }
        memset(message, 'A', params->message_size);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (size_t i = 0; i < params->message_count; ++i) {
            // Wait for the reader to signal it is ready
            while (*read_flag == 0) {
                usleep(1); // Prevent CPU spinning
            }

            memcpy((char*)data_addr + i * params->message_size, message, params->message_size);

            // Signal to the reader that data is ready
            *read_flag = 0;
            __sync_synchronize();
            *write_flag = 1;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_sec = end.tv_sec - start.tv_sec;
        double elapsed_nsec = end.tv_nsec - start.tv_nsec;
        double elapsed = elapsed_sec + elapsed_nsec / 1e9;

        double throughput = (double)(params->message_size * params->message_count) / elapsed;

        printf("Elapsed time: %f seconds\n", elapsed);
        printf("Throughput: %f bytes/second\n", throughput);
        printf("Test finished.\n");
        printf("---\n");

        free(message);

        wait(NULL);

        munmap(addr, total_size);
        shm_unlink(shm_name);
        close(fd);
    }
}

// src/ipc_shared_memory.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

void ipc_shared_memory(const ipc_params_t* params) {
    const char* shm_name = "/ipc_shm";
    const char* sem_full_name = "/ipc_sem_full";
    const char* sem_empty_name = "/ipc_sem_empty";
    size_t total_size = params->message_size * params->message_count;

    // Створення або відкриття об'єкта спільної пам'яті
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Встановлення розміру спільної пам'яті
    if (ftruncate(fd, total_size) == -1) {
        perror("ftruncate");
        shm_unlink(shm_name);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Відображення спільної пам'яті в адресний простір
    void* addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        shm_unlink(shm_name);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Створення семафорів для синхронізації
    sem_t* sem_full = sem_open(sem_full_name, O_CREAT | O_EXCL, 0666, 0);
    sem_t* sem_empty = sem_open(sem_empty_name, O_CREAT | O_EXCL, 0666, params->message_count);

    if (sem_full == SEM_FAILED || sem_empty == SEM_FAILED) {
        perror("sem_open");
        munmap(addr, total_size);
        shm_unlink(shm_name);
        sem_unlink(sem_full_name);
        sem_unlink(sem_empty_name);
        close(fd);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        munmap(addr, total_size);
        shm_unlink(shm_name);
        sem_unlink(sem_full_name);
        sem_unlink(sem_empty_name);
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Дочірній процес (читач)
        char* buffer = malloc(params->message_size);
        if (!buffer) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < params->message_count; ++i) {
            sem_wait(sem_full); // Очікуємо, поки з'явиться повідомлення

            memcpy(buffer, (char*)addr + i * params->message_size, params->message_size);

            sem_post(sem_empty); // Повідомляємо, що місце звільнилося

            // Обробка повідомлення при необхідності
        }

        free(buffer);

        munmap(addr, total_size);
        close(fd);

        // Закриття семафорів
        sem_close(sem_full);
        sem_close(sem_empty);

        exit(EXIT_SUCCESS);
    } else { // Батьківський процес (писач)
        char* message = malloc(params->message_size);
        if (!message) {
            perror("malloc");
            munmap(addr, total_size);
            shm_unlink(shm_name);
            sem_unlink(sem_full_name);
            sem_unlink(sem_empty_name);
            close(fd);
            exit(EXIT_FAILURE);
        }
        memset(message, 'A', params->message_size);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (size_t i = 0; i < params->message_count; ++i) {
            sem_wait(sem_empty); // Очікуємо, поки звільниться місце

            memcpy((char*)addr + i * params->message_size, message, params->message_size);

            sem_post(sem_full); // Повідомляємо, що повідомлення доступне
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_sec = end.tv_sec - start.tv_sec;
        double elapsed_nsec = end.tv_nsec - start.tv_nsec;
        double elapsed = elapsed_sec + elapsed_nsec / 1e9;

        printf("Elapsed time: %f seconds\n", elapsed);

        // Розрахунок пропускної здатності
        double throughput = (double)(params->message_size * params->message_count) / elapsed;
        printf("Throughput: %f bytes/second\n", throughput);

        free(message);

        // Очікуємо завершення дочірнього процесу
        wait(NULL);

        munmap(addr, total_size);
        shm_unlink(shm_name);
        close(fd);

        // Закриття та видалення семафорів
        sem_close(sem_full);
        sem_close(sem_empty);
        sem_unlink(sem_full_name);
        sem_unlink(sem_empty_name);
    }
}

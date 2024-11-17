// src/ipc_file.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>

void ipc_file(const ipc_params_t* params) {
    const char* data_file = "ipc_data.bin";
    const char* ready_file = "ipc_ready.flag";

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Дочірній процес (читач)
        // Очікуємо появи файлу-мітки
        while (access(ready_file, F_OK) == -1) {
            // Невелика затримка, щоб не перевантажувати CPU
            usleep(1000);
        }

        int fd = open(data_file, O_RDONLY);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        char* buffer = malloc(params->message_size);
        if (!buffer) {
            perror("malloc");
            close(fd);
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_read = read(fd, buffer, params->message_size);
            if (bytes_read <= 0) {
                perror("read");
                free(buffer);
                close(fd);
                exit(EXIT_FAILURE);
            }
            // Обробка повідомлення при необхідності
        }

        free(buffer);
        close(fd);

        // Видаляємо файли після читання
        unlink(data_file);
        unlink(ready_file);

        exit(EXIT_SUCCESS);
    } else { // Батьківський процес (писач)
        int fd = open(data_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        char* message = malloc(params->message_size);
        if (!message) {
            perror("malloc");
            close(fd);
            exit(EXIT_FAILURE);
        }
        memset(message, 'A', params->message_size);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_written = write(fd, message, params->message_size);
            if (bytes_written <= 0) {
                perror("write");
                free(message);
                close(fd);
                exit(EXIT_FAILURE);
            }
        }

        fsync(fd); // Синхронізуємо дані з диском
        close(fd);

        // Створюємо файл-мітку
        int flag_fd = open(ready_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (flag_fd == -1) {
            perror("open ready_file");
            free(message);
            exit(EXIT_FAILURE);
        }
        close(flag_fd);

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_sec = end.tv_sec - start.tv_sec;
        double elapsed_nsec = end.tv_nsec - start.tv_nsec;
        double elapsed = elapsed_sec + elapsed_nsec / 1e9;

        printf("Elapsed time: %f seconds\n", elapsed);

        // Розрахунок пропускної здатності
        double throughput = (double)(params->message_size * params->message_count) / elapsed;
        printf("Throughput: %f bytes/second\n", throughput);

        free(message);

        // Чекаємо на завершення дочірнього процесу
        wait(NULL);
    }
}

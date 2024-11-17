// src/ipc_pipe.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>

void ipc_pipe(const ipc_params_t* params) {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Дочірній процес (читач)
        close(fd[1]); // Закриваємо запис
        char* buffer = malloc(params->message_size);
        if (!buffer) {
            perror("malloc");
            close(fd[0]);
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_read = read(fd[0], buffer, params->message_size);
            if (bytes_read <= 0) {
                perror("read");
                free(buffer);
                close(fd[0]);
                exit(EXIT_FAILURE);
            }
            // Обробка повідомлення при необхідності
        }

        free(buffer);
        close(fd[0]);
        exit(EXIT_SUCCESS);
    } else { // Батьківський процес (писач)
        close(fd[0]); // Закриваємо читання

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        char* message = malloc(params->message_size);
        if (!message) {
            perror("malloc");
            close(fd[1]);
            exit(EXIT_FAILURE);
        }
        memset(message, 'A', params->message_size);

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_written = write(fd[1], message, params->message_size);
            if (bytes_written <= 0) {
                perror("write");
                free(message);
                close(fd[1]);
                exit(EXIT_FAILURE);
            }
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
        close(fd[1]);

        // Чекаємо на завершення дочірнього процесу
        wait(NULL);
    }
}

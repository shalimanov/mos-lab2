// src/ipc_measure_capacity_pipe.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>

#define INITIAL_MSG_COUNT 1
#define MAX_MSG_COUNT 10000
#define STEP_MSG_COUNT 100

void ipc_measure_capacity_pipe() {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Встановлюємо write end у неблокуючий режим
    int flags = fcntl(fd[1], F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        close(fd[0]);
        close(fd[1]);
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd[1], F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        close(fd[0]);
        close(fd[1]);
        exit(EXIT_FAILURE);
    }

    size_t message_size = 64; // Початковий розмір повідомлення
    size_t message_count = INITIAL_MSG_COUNT;

    printf("Вимірювання ємності каналу (Pipe)\n");
    printf("Розмір повідомлення: %zu байт\n", message_size);

    while (message_count <= MAX_MSG_COUNT) {
        int success = 1;
        for (size_t i = 0; i < message_count; ++i) {
            char* message = malloc(message_size);
            if (!message) {
                perror("malloc");
                success = 0;
                break;
            }
            memset(message, 'A', message_size);

            ssize_t bytes_written = write(fd[1], message, message_size);
            if (bytes_written == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Канал переповнений
                    success = 0;
                    free(message);
                    break;
                } else {
                    perror("write");
                    success = 0;
                    free(message);
                    break;
                }
            }
            free(message);
        }

        if (success) {
            printf("Ємність каналу: %zu повідомлень\n", message_count);
            message_count += STEP_MSG_COUNT;
        } else {
            printf("Максимальна ємність каналу до %zu повідомлень\n", message_count - STEP_MSG_COUNT);
            break;
        }
    }

    close(fd[0]);
    close(fd[1]);
}

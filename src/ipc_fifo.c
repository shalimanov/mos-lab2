// src/ipc_fifo.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

void ipc_fifo(const ipc_params_t* params) {
    const char* fifo_name = "/tmp/ipc_fifo";

    if (mkfifo(fifo_name, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        unlink(fifo_name);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        int fd = open(fifo_name, O_RDONLY);
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
        }

        free(buffer);
        close(fd);
        exit(EXIT_SUCCESS);
    } else {
        int fd = open(fifo_name, O_WRONLY);
        if (fd == -1) {
            perror("open");
            unlink(fifo_name);
            exit(EXIT_FAILURE);
        }

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        char* message = malloc(params->message_size);
        if (!message) {
            perror("malloc");
            close(fd);
            unlink(fifo_name);
            exit(EXIT_FAILURE);
        }
        memset(message, 'A', params->message_size);

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_written = write(fd, message, params->message_size);
            if (bytes_written <= 0) {
                perror("write");
                free(message);
                close(fd);
                unlink(fifo_name);
                exit(EXIT_FAILURE);
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_sec = end.tv_sec - start.tv_sec;
        double elapsed_nsec = end.tv_nsec - start.tv_nsec;
        double elapsed = elapsed_sec + elapsed_nsec / 1e9;

        printf("Elapsed time: %f seconds\n", elapsed);

        double throughput = (double)(params->message_size * params->message_count) / elapsed;
        printf("Throughput: %f bytes/second\n", throughput);

        free(message);
        close(fd);

        wait(NULL);
        unlink(fifo_name);
    }
}

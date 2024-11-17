// src/ipc_unix_socket.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

void ipc_unix_socket(const ipc_params_t* params) {
    const char* socket_path = "/tmp/ipc_unix_socket";
    int server_fd, client_fd;
    pid_t pid;

    unlink(socket_path);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        struct sockaddr_un addr;
        client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client_fd == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

        while (connect(client_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
            if (errno == ENOENT) {
                usleep(1000);
                continue;
            } else {
                perror("connect");
                close(client_fd);
                exit(EXIT_FAILURE);
            }
        }

        char* buffer = malloc(params->message_size);
        if (!buffer) {
            perror("malloc");
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_received = recv(client_fd, buffer, params->message_size, MSG_WAITALL);
            if (bytes_received <= 0) {
                perror("recv");
                free(buffer);
                close(client_fd);
                exit(EXIT_FAILURE);
            }
        }

        free(buffer);
        close(client_fd);
        exit(EXIT_SUCCESS);
    } else {
        struct sockaddr_un addr;
        server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
            perror("bind");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 1) == -1) {
            perror("listen");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        int conn_fd = accept(server_fd, NULL, NULL);
        if (conn_fd == -1) {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        char* message = malloc(params->message_size);
        if (!message) {
            perror("malloc");
            close(conn_fd);
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        memset(message, 'A', params->message_size);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_sent = send(conn_fd, message, params->message_size, 0);
            if (bytes_sent <= 0) {
                perror("send");
                free(message);
                close(conn_fd);
                close(server_fd);
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
        close(conn_fd);
        close(server_fd);
        unlink(socket_path);

        wait(NULL);
    }
}

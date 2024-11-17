// src/ipc_measure_capacity_unix_socket.c
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
#include <fcntl.h>

#define INITIAL_MSG_COUNT 1
#define MAX_MSG_COUNT 100000
#define STEP_MSG_COUNT 1000

void ipc_measure_capacity_unix_socket() {
    const char* socket_path = "/tmp/ipc_unix_socket_capacity";
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
            perror("socket (client)");
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
        printf("Клієнт: Підключено до сервера.\n");

        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl F_GETFL");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl F_SETFL");
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        size_t message_size = 64;
        size_t message_count = 0;

        while (1) {
            char buffer[message_size];
            ssize_t bytes_received = recv(client_fd, buffer, message_size, 0);
            if (bytes_received > 0) {
                message_count++;
            } else if (bytes_received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                printf("Клієнт: Всі повідомлення отримані.\n");
                break;
            } else if (bytes_received == 0) {
                printf("Клієнт: З'єднання закрито сервером.\n");
                break;
            } else {
                perror("recv");
                close(client_fd);
                exit(EXIT_FAILURE);
            }
        }

        printf("Максимальна ємність UNIX сокета: %zu повідомлень\n", message_count);

        close(client_fd);
        exit(EXIT_SUCCESS);
    } else {
        struct sockaddr_un addr;
        server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("socket (server)");
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
        printf("Сервер: Сокет прослуховується.\n");

        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        printf("Сервер: Клієнт підключився.\n");

        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl F_GETFL (server)");
            close(client_fd);
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl F_SETFL (server)");
            close(client_fd);
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        size_t message_size = 64;
        size_t message_count = 0;
        int capacity_found = 0;

        struct timespec timeout;
        clock_gettime(CLOCK_MONOTONIC, &timeout);
        timeout.tv_sec += 5;

        while (!capacity_found) {
            char* message = malloc(message_size);
            if (!message) {
                perror("malloc");
                close(client_fd);
                close(server_fd);
                exit(EXIT_FAILURE);
            }
            memset(message, 'A', message_size);

            ssize_t bytes_sent = send(client_fd, message, message_size, 0);
            if (bytes_sent == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    printf("Сервер: Буфер сокета переповнений. Максимальна ємність UNIX сокета: %zu повідомлень\n", message_count);
                    capacity_found = 1;
                } else {
                    perror("send");
                    free(message);
                    close(client_fd);
                    close(server_fd);
                    exit(EXIT_FAILURE);
                }
            } else {
                message_count++;
                if (message_count % 10000 == 0) {
                    printf("Сервер: Відправлено %zu повідомлень\n", message_count);
                }
            }

            free(message);

            struct timespec current_time;
            clock_gettime(CLOCK_MONOTONIC, &current_time);
            if ((current_time.tv_sec > timeout.tv_sec) ||
                (current_time.tv_sec == timeout.tv_sec && current_time.tv_nsec > timeout.tv_nsec)) {
                printf("Сервер: Таймаут досягнуто. Максимальна ємність UNIX сокета: %zu повідомлень\n", message_count);
                capacity_found = 1;
            }
        }

        close(client_fd);
        close(server_fd);
        unlink(socket_path);

        wait(NULL);
    }
}

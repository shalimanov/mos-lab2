// src/ipc_message_queue.c
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>

void ipc_message_queue(const ipc_params_t* params) {
    const char* mq_name = "/ipc_mq";
    mqd_t mqd;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = params->message_size;
    attr.mq_curmsgs = 0;

    // Видаляємо чергу, якщо вона вже існує
    mq_unlink(mq_name);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Дочірній процес (читач)
        mqd = mq_open(mq_name, O_RDONLY | O_CREAT, 0666, &attr);
        if (mqd == (mqd_t)-1) {
            perror("mq_open (reader)");
            exit(EXIT_FAILURE);
        }

        char* buffer = malloc(params->message_size);
        if (!buffer) {
            perror("malloc");
            mq_close(mqd);
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < params->message_count; ++i) {
            ssize_t bytes_read = mq_receive(mqd, buffer, params->message_size, NULL);
            if (bytes_read == -1) {
                perror("mq_receive");
                free(buffer);
                mq_close(mqd);
                exit(EXIT_FAILURE);
            }
            // Обробка повідомлення при необхідності
        }

        free(buffer);
        mq_close(mqd);
        exit(EXIT_SUCCESS);
    } else { // Батьківський процес (писач)
        mqd = mq_open(mq_name, O_WRONLY | O_CREAT, 0666, &attr);
        if (mqd == (mqd_t)-1) {
            perror("mq_open (writer)");
            exit(EXIT_FAILURE);
        }

        char* message = malloc(params->message_size);
        if (!message) {
            perror("malloc");
            mq_close(mqd);
            exit(EXIT_FAILURE);
        }
        memset(message, 'A', params->message_size);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (size_t i = 0; i < params->message_count; ++i) {
            if (mq_send(mqd, message, params->message_size, 0) == -1) {
                perror("mq_send");
                free(message);
                mq_close(mqd);
                mq_unlink(mq_name);
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
        mq_close(mqd);

        // Чекаємо на завершення дочірнього процесу
        wait(NULL);

        // Видаляємо чергу повідомлень
        mq_unlink(mq_name);
    }
}

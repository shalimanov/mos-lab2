// ipc_measure_capacity_message_queue.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define MAX_MSG_COUNT 20
#define MSG_SIZE 64

void ipc_measure_capacity_message_queue(FILE *csv_file) {
    const char* mq_name = "/ipc_mq_capacity_test";
    mqd_t mqd;
    struct mq_attr attr;

    memset(&attr, 0, sizeof(attr));
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG_COUNT;
    attr.mq_msgsize = MSG_SIZE;

    if (mq_unlink(mq_name) == -1 && errno != ENOENT) {
        perror("mq_unlink");
    }

    mqd = mq_open(mq_name, O_CREAT | O_WRONLY | O_NONBLOCK, 0666, &attr);
    if (mqd == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    struct mq_attr actual_attr;
    if (mq_getattr(mqd, &actual_attr) == -1) {
        perror("mq_getattr");
        mq_close(mqd);
        exit(EXIT_FAILURE);
    }

    printf("Actual mq_maxmsg: %ld\n", actual_attr.mq_maxmsg);
    printf("Actual mq_msgsize: %ld\n", actual_attr.mq_msgsize);

    if (actual_attr.mq_maxmsg < MAX_MSG_COUNT || actual_attr.mq_msgsize < MSG_SIZE) {
        printf("Невідповідність атрибутів черги повідомлень.\n");
        mq_close(mqd);
        exit(EXIT_FAILURE);
    }

    size_t message_count = 0;
    struct timespec start_time, end_time;
    double elapsed_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (size_t i = 1; i <= MAX_MSG_COUNT; ++i) {
        char message[MSG_SIZE];
        memset(message, 'A', sizeof(message));
        if (mq_send(mqd, message, sizeof(message), 0) == -1) {
            if (errno == EAGAIN) {
                printf("mq_send: Черга переповнена (EAGAIN) при спробі відправити повідомлення %zu\n", i);
                break;
            } else {
                printf("mq_send: Помилка %d при спробі відправити повідомлення %zu\n", errno, i);
                break;
            }
        }
        printf("Відправлено повідомлення %zu\n", i);
        message_count = i;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                   (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    double throughput = (double)(message_count * MSG_SIZE) / elapsed_time;

    fprintf(csv_file, "message_queue,%d,%zu,%.6f,%.6f\n", 
        MSG_SIZE, 
        message_count, 
        elapsed_time, 
        throughput
    );

    printf("Максимальна ємність черги повідомлень до %zu повідомлень\n", message_count);

    mq_close(mqd);
    mq_unlink(mq_name);
}

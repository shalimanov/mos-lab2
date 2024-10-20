#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h> // Для usleep()
#include "ipc_mmap.h"
#include "ipc_shm.h"
#include "ipc_msgqueue.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Використання: %s <метод_ipc> <розмір_повідомлення> <кількість_повідомлень> <режим_mmap>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *ipc_method = argv[1];
    size_t message_size = atoi(argv[2]);
    size_t message_count = atoi(argv[3]);
    int mmap_mode = 0;

    if (strcmp(ipc_method, "mmap") == 0) {
        if (strcmp(argv[4], "MAP_SHARED") == 0) {
            mmap_mode = MAP_SHARED;
        } else {
            printf("Невідомий режим mmap: %s. Підтримується тільки MAP_SHARED.\n", argv[4]);
            return EXIT_FAILURE;
        }
        if (ipc_mmap_init(mmap_mode, message_size) != 0) {
            return EXIT_FAILURE;
        }
    } else if (strcmp(ipc_method, "shm") == 0) {
        if (ipc_shm_init(message_size) != 0) {
            return EXIT_FAILURE;
        }
    } else if (strcmp(ipc_method, "msgqueue") == 0) {
        if (ipc_msgqueue_init() != 0) {
            return EXIT_FAILURE;
        }
    } else {
        printf("Невідомий метод IPC: %s. Використовуйте mmap, shm або msgqueue.\n", ipc_method);
        return EXIT_FAILURE;
    }

    char *message = malloc(message_size);
    if (message == NULL) {
        log_error("Не вдалося виділити пам'ять для повідомлення");
        if (strcmp(ipc_method, "mmap") == 0) ipc_mmap_cleanup();
        if (strcmp(ipc_method, "shm") == 0) ipc_shm_cleanup();
        if (strcmp(ipc_method, "msgqueue") == 0) ipc_msgqueue_cleanup();
        return EXIT_FAILURE;
    }
    memset(message, 'A', message_size);

    struct timespec start_time;
    double total_latency = 0;

    for (size_t i = 0; i < 10000; i++) {
        start_timer(&start_time);
        if (strcmp(ipc_method, "mmap") == 0) {
            ipc_mmap_send(message, message_size);
        } else if (strcmp(ipc_method, "shm") == 0) {
            ipc_shm_send(message, message_size);
        } else if (strcmp(ipc_method, "msgqueue") == 0) {
            ipc_msgqueue_send(message, message_size);
            usleep(100); // Додаємо затримку 100 мікросекунд між відправками
        }
        double latency = end_timer(&start_time);
        total_latency += latency;
    }

    double average_latency = total_latency / 10000;

    start_timer(&start_time);
    for (size_t i = 0; i < message_count; i++) {
        if (strcmp(ipc_method, "mmap") == 0) {
            ipc_mmap_send(message, message_size);
        } else if (strcmp(ipc_method, "shm") == 0) {
            ipc_shm_send(message, message_size);
        } else if (strcmp(ipc_method, "msgqueue") == 0) {
            ipc_msgqueue_send(message, message_size);
            usleep(100); // Затримка 100 мікросекунд
        }
    }
    double total_time = end_timer(&start_time);

    Stats stats;
    stats.latency = average_latency;
    stats.throughput = (message_size * message_count) / total_time;
    stats.channel_capacity = (strcmp(ipc_method, "mmap") == 0) ? ipc_mmap_get_capacity() :
                             (strcmp(ipc_method, "shm") == 0) ? ipc_shm_get_capacity() : ipc_msgqueue_get_capacity();

    const char *result_file = "results/summary.csv";
    save_results_to_csv(result_file, &stats, message_size, message_count, ipc_method);

    free(message);
    if (strcmp(ipc_method, "mmap") == 0) ipc_mmap_cleanup();
    if (strcmp(ipc_method, "shm") == 0) ipc_shm_cleanup();
    if (strcmp(ipc_method, "msgqueue") == 0) ipc_msgqueue_cleanup();

    return EXIT_SUCCESS;
}

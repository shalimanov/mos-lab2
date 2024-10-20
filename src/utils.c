#include "utils.h"
#include <stdarg.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Помилка: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void start_timer(struct timespec *start) {
#ifdef __MACH__
    // macOS не підтримує clock_gettime, тому використовуємо альтернативу
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    start->tv_sec = mts.tv_sec;
    start->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_MONOTONIC, start);
#endif
}

double end_timer(struct timespec *start) {
    struct timespec end;
#ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    end.tv_sec = mts.tv_sec;
    end.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_MONOTONIC, &end);
#endif
    double elapsed = (end.tv_sec - start->tv_sec) + (end.tv_nsec - start->tv_nsec) / 1e9;
    return elapsed;
}

void save_results_to_csv(const char *filename, Stats *stats, size_t message_size, size_t message_count, const char *ipc_method) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        log_error("Не вдалося відкрити файл %s для запису", filename);
        return;
    }
    fprintf(file, "%s,%zu,%zu,%f,%f,%zu\n", ipc_method, message_size, message_count, stats->latency, stats->throughput, stats->channel_capacity);
    fclose(file);
}

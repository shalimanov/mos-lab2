#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <time.h>

typedef struct {
    double latency;
    double throughput;
    size_t channel_capacity;
} Stats;

void log_error(const char *format, ...);
void start_timer(struct timespec *start);
double end_timer(struct timespec *start);
void save_results_to_csv(const char *filename, Stats *stats, size_t message_size, size_t message_count, const char *ipc_method);

#endif // UTILS_H

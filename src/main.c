// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipc.h"

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s IPC_METHOD MESSAGE_SIZE MESSAGE_COUNT MMAP_MODE\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* ipc_method_name = argv[1];
    ipc_params_t params;
    params.message_size = strtoul(argv[2], NULL, 10);
    params.message_count = strtoul(argv[3], NULL, 10);

    // Парсинг режиму mmap
    if (strcmp(argv[4], "shared") == 0) {
        params.mmap_mode = IPC_MMAP_SHARED;
    } else if (strcmp(argv[4], "private") == 0) {
        params.mmap_mode = IPC_MMAP_PRIVATE;
    } else {
        fprintf(stderr, "Unknown mmap mode: %s\n", argv[4]);
        return EXIT_FAILURE;
    }

    ipc_method_t ipc_method = NULL;

    if (strcmp(ipc_method_name, "mmap") == 0) {
        ipc_method = ipc_mmap;
    }
    // Додайте інші методи за потреби

    if (ipc_method == NULL) {
        fprintf(stderr, "Unknown IPC method: %s\n", ipc_method_name);
        return EXIT_FAILURE;
    }

    ipc_method(&params);

    return EXIT_SUCCESS;
}

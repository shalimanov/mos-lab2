// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipc.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s IPC_METHOD [OTHER_PARAMETERS]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* ipc_method_name = argv[1];
    ipc_method_t ipc_method = NULL;
    ipc_params_t params;

    if (strcmp(ipc_method_name, "mmap") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Usage for mmap: %s mmap MESSAGE_SIZE MESSAGE_COUNT MMAP_MODE(shared/private)\n", argv[0]);
            return EXIT_FAILURE;
        }
        params.message_size = strtoul(argv[2], NULL, 10);
        params.message_count = strtoul(argv[3], NULL, 10);
        if (strcmp(argv[4], "shared") == 0) {
            params.mmap_mode = IPC_MMAP_SHARED;
        } else if (strcmp(argv[4], "private") == 0) {
            params.mmap_mode = IPC_MMAP_PRIVATE;
        } else {
            fprintf(stderr, "Unknown mmap mode: %s\n", argv[4]);
            return EXIT_FAILURE;
        }
        ipc_method = ipc_mmap;
    }
    else if (strcmp(ipc_method_name, "shared_memory") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage for shared_memory: %s shared_memory MESSAGE_SIZE MESSAGE_COUNT\n", argv[0]);
            return EXIT_FAILURE;
        }
        params.message_size = strtoul(argv[2], NULL, 10);
        params.message_count = strtoul(argv[3], NULL, 10);
        ipc_method = ipc_shared_memory;
    }
    else if (strcmp(ipc_method_name, "pipe") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage for pipe: %s pipe MESSAGE_SIZE MESSAGE_COUNT\n", argv[0]);
            return EXIT_FAILURE;
        }
        params.message_size = strtoul(argv[2], NULL, 10);
        params.message_count = strtoul(argv[3], NULL, 10);
        ipc_method = ipc_pipe;
    }
    else if (strcmp(ipc_method_name, "fifo") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage for fifo: %s fifo MESSAGE_SIZE MESSAGE_COUNT\n", argv[0]);
            return EXIT_FAILURE;
        }
        params.message_size = strtoul(argv[2], NULL, 10);
        params.message_count = strtoul(argv[3], NULL, 10);
        ipc_method = ipc_fifo;
    }
    else if (strcmp(ipc_method_name, "file") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage for file: %s file MESSAGE_SIZE MESSAGE_COUNT\n", argv[0]);
            return EXIT_FAILURE;
        }
        params.message_size = strtoul(argv[2], NULL, 10);
        params.message_count = strtoul(argv[3], NULL, 10);
        ipc_method = ipc_file;
    }
    else if (strcmp(ipc_method_name, "message_queue") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage for message_queue: %s message_queue MESSAGE_SIZE MESSAGE_COUNT\n", argv[0]);
            return EXIT_FAILURE;
        }
        params.message_size = strtoul(argv[2], NULL, 10);
        params.message_count = strtoul(argv[3], NULL, 10);
        ipc_method = ipc_message_queue;
    }
    else if (strcmp(ipc_method_name, "unix_socket") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage for unix_socket: %s unix_socket MESSAGE_SIZE MESSAGE_COUNT\n", argv[0]);
            return EXIT_FAILURE;
        }
        params.message_size = strtoul(argv[2], NULL, 10);
        params.message_count = strtoul(argv[3], NULL, 10);
        ipc_method = ipc_unix_socket;
    }
    else if (strcmp(ipc_method_name, "measure_capacity_message_queue") == 0) {
        ipc_measure_capacity_message_queue();
        return EXIT_SUCCESS;
    }
    else if (strcmp(ipc_method_name, "measure_capacity_pipe") == 0) {
        ipc_measure_capacity_pipe();
        return EXIT_SUCCESS;
    }
    else if (strcmp(ipc_method_name, "measure_capacity_unix_socket") == 0) {
        ipc_measure_capacity_unix_socket();
        return EXIT_SUCCESS;
    }
    else {
        fprintf(stderr, "Unknown IPC method: %s\n", ipc_method_name);
        return EXIT_FAILURE;
    }

    if (ipc_method) {
        ipc_method(&params);
    }

    return EXIT_SUCCESS;
}

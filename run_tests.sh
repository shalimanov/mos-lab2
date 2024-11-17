#!/bin/bash

make clean && make

IPC_METHODS=("mmap" "shared_memory" "pipe" "fifo" "file" "message_queue" "unix_socket")
MESSAGE_SIZES=(64 1024)
MESSAGE_COUNTS=(100 1000)
MMAP_MODES=("shared" "private")

echo "---"
echo "Start testing."

for IPC_METHOD in "${IPC_METHODS[@]}"; do
  for MESSAGE_SIZE in "${MESSAGE_SIZES[@]}"; do
    for MESSAGE_COUNT in "${MESSAGE_COUNTS[@]}"; do
      if [ "$IPC_METHOD" == "mmap" ]; then
        for MMAP_MODE in "${MMAP_MODES[@]}"; do
          echo "Testing IPC_METHOD=$IPC_METHOD, MESSAGE_SIZE=$MESSAGE_SIZE, MESSAGE_COUNT=$MESSAGE_COUNT, MMAP_MODE=$MMAP_MODE"
          ./ipc_compare $IPC_METHOD $MESSAGE_SIZE $MESSAGE_COUNT $MMAP_MODE
          echo "Test finished."
          echo "---"
        done
      else
        echo "Testing IPC_METHOD=$IPC_METHOD, MESSAGE_SIZE=$MESSAGE_SIZE, MESSAGE_COUNT=$MESSAGE_COUNT"
        ./ipc_compare $IPC_METHOD $MESSAGE_SIZE $MESSAGE_COUNT
        echo "Test finished."
        echo "---"
      fi
    done
  done
done

echo "Measuring Channel Capacity for message_queue"
./ipc_compare measure_capacity_message_queue
echo "Measurement finished."
echo "---"

echo "Measuring Channel Capacity for pipe"
./ipc_compare measure_capacity_pipe
echo "Measurement finished."
echo "---"

echo "Measuring Channel Capacity for unix_socket"
./ipc_compare measure_capacity_unix_socket
echo "Measurement finished."
echo "---"

echo "Testing has been finished."
echo "---"

#!/bin/bash

make clean && make

IPC_METHODS=("mmap")
MESSAGE_SIZES=(64 1024)
MESSAGE_COUNTS=(100 1000)
MMAP_MODES=("shared" "private")

echo "---"
echo "Start testing."

for IPC_METHOD in "${IPC_METHODS[@]}"; do
  for MESSAGE_SIZE in "${MESSAGE_SIZES[@]}"; do
    for MESSAGE_COUNT in "${MESSAGE_COUNTS[@]}"; do
      for MMAP_MODE in "${MMAP_MODES[@]}"; do
        echo "Testing IPC_METHOD=$IPC_METHOD, MESSAGE_SIZE=$MESSAGE_SIZE, MESSAGE_COUNT=$MESSAGE_COUNT, MMAP_MODE=$MMAP_MODE"
        ./ipc_compare $IPC_METHOD $MESSAGE_SIZE $MESSAGE_COUNT $MMAP_MODE
        echo "Test finished."
        echo "---"
      done
    done
  done
done

echo "Testing has been finished."
echo "---"

#!/bin/bash

mkdir -p results

MESSAGE_SIZES=(128 256 512 1024)
MESSAGE_COUNTS=(100 1000 10000)
MMAP_MODE="MAP_SHARED"
IPC_METHODS=("mmap" "shm" "msgqueue")

RESULT_FILE="results/summary.csv"
echo "Method,MessageSize,MessageCount,Mode,Latency (sec),Throughput (bytes/sec),Capacity" > $RESULT_FILE

for METHOD in "${IPC_METHODS[@]}"; do
    for SIZE in "${MESSAGE_SIZES[@]}"; do
        for COUNT in "${MESSAGE_COUNTS[@]}"; do
            echo "Тестування з методом ${METHOD}, розміром повідомлення ${SIZE}, кількістю повідомлень ${COUNT}"
            if [ "$METHOD" == "mmap" ]; then
                ./ipc_test $METHOD $SIZE $COUNT $MMAP_MODE >> $RESULT_FILE
            else
                ./ipc_test $METHOD $SIZE $COUNT dummy >> $RESULT_FILE
            fi
        done
    done
done

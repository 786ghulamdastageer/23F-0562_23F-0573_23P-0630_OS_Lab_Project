#!/bin/bash

ADMISSIONS_PID=$(pgrep -f "admissions")

if [ -n "$ADMISSIONS_PID" ]; then
    echo "Shutting down admissions manager..."
    kill -SIGTERM $ADMISSIONS_PID
    sleep 2
fi

if [ -f /tmp/discharge_fifo ]; then
    rm -f /tmp/discharge_fifo
    echo "FIFO removed"
fi

SHM_IDS=$(ipcs -m | grep $USER | awk '{print $2}')
for ID in $SHM_IDS; do
    ipcrm -m $ID 2>/dev/null
done

echo "All IPC resources cleaned"
echo "=========================================="
echo "HOSPITAL SHUT DOWN"
echo "=========================================="
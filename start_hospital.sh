#!/bin/bash

make clean > /dev/null 2>&1
make all > /dev/null 2>&1

if [ ! -f ./src/admissions ]; then
    echo "Compilation failed"
    exit 1
fi

if [ -f /tmp/discharge_fifo ]; then
    rm -f /tmp/discharge_fifo
fi

mkfifo /tmp/discharge_fifo

echo "=========================================="
echo "HOSPITAL PATIENT TRIAGE SYSTEM"
echo "=========================================="
echo "ICU Beds: 4"
echo "Isolation Beds: 4"
echo "General Ward Beds: 12"
echo "Total Capacity: 20 Beds"
echo "=========================================="

./src/admissions $1 &

echo "Admissions Manager PID: $!"
echo "Hospital is ready"
echo "=========================================="
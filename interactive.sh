#!/bin/bash

while true; do
    echo ""
    echo "=========================================="
    echo "Enter patient name (or 'quit' to exit): "
    read name
    if [ "$name" = "quit" ]; then break; fi
    
    echo "Enter age: "
    read age
    
    echo "Enter severity (1-10): "
    read severity
    
    ./scripts/triage.sh "$name" $age $severity | ./src/admissions
done
echo "Session ended."
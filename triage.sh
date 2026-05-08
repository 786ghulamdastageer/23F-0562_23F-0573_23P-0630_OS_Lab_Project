#!/bin/bash

if [ $# -ne 3 ]; then
    echo "Usage: $0 <patient_name> <age> <symptom_severity(1-10)>"
    exit 1
fi

PATIENT_NAME=$1
AGE=$2
SEVERITY=$3

if [ $AGE -lt 0 ] || [ $AGE -gt 120 ]; then
    echo "Error: Age must be between 0 and 120"
    exit 1
fi

if [ $SEVERITY -lt 1 ] || [ $SEVERITY -gt 10 ]; then
    echo "Error: Severity must be between 1 and 10"
    exit 1
fi

if [ $SEVERITY -ge 8 ]; then
    PRIORITY=1
elif [ $SEVERITY -ge 6 ]; then
    PRIORITY=2
elif [ $SEVERITY -ge 4 ]; then
    PRIORITY=3
elif [ $SEVERITY -ge 2 ]; then
    PRIORITY=4
else
    PRIORITY=5
fi

PATIENT_ID=$$

echo "$PATIENT_ID|$PATIENT_NAME|$AGE|$SEVERITY|$PRIORITY"

exit 0
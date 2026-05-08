CC = gcc
CFLAGS = -Wall -pthread
TARGET_DIR = src
LOGS_DIR = logs

all:
	mkdir -p $(LOGS_DIR)
	$(CC) $(TARGET_DIR)/admissions.c $(TARGET_DIR)/scheduling.c $(TARGET_DIR)/memory_allocator.c -o $(TARGET_DIR)/admissions $(CFLAGS)
	$(CC) $(TARGET_DIR)/patient_simulator.c -o $(TARGET_DIR)/patient_simulator $(CFLAGS)

clean:
	rm -f $(TARGET_DIR)/admissions $(TARGET_DIR)/patient_simulator
	rm -f /tmp/discharge_fifo
	rm -f $(LOGS_DIR)/*.txt

run:
	./scripts/start_hospital.sh

stop:
	./scripts/stop_hospital.sh

test:
	./scripts/start_hospital.sh &
	sleep 2
	for i in {1..20}; do
		echo "100$$i|Patient$$i|30|$$((RANDOM % 10 + 1))|$$((RANDOM % 5 + 1))" | ./src/admissions --strategy best
		sleep 1
	done
	sleep 30
	./scripts/stop_hospital.sh

.PHONY: all clean run stop test
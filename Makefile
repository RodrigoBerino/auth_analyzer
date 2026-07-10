CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -Isrc
SRC = src/main.c src/auth_analyzer.c src/split.c src/hashtable.c src/map.c src/utils/mmap_utils.c src/utils/log_parser.c src/utils/report.c src/utils/timer_utils.c
TARGET = authanalyzer

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

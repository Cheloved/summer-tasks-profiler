# Конфигурация
CC := g++
BUILD_DIR ?= build
TEST_DIR=test_files

# Флаги
CFLAGS := -Wall -Wextra -pedantic -O2 -g

# Исходные файлы
C_SRC := $(wildcard *.cpp)

# Компиляция всех .cpp файлов в build/task-1.out
all: always $(BUILD_DIR)/task-2.out

$(BUILD_DIR)/task-2.out:
	$(CC) $(CFLAGS) $(C_SRC) -o $@

test:
	./run_tests.sh

# Всегда создавать директорию build/
always:
	mkdir -p $(BUILD_DIR)

# Очистка всех файлов сборки
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.o

clean_tests:
	rm -rf $(TEST_DIR) actual_output.txt expected_output.txt hash_file.py

# Не считать all, clean и always за имена файлов при вызове make
.PHONY: all clean always

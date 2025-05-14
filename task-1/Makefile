# Конфигурация
CC := g++
BUILD_DIR ?= build

# Флаги
CFLAGS := -Wall -Wextra -pedantic -O2 -g

# Исходные файлы
C_SRC := $(wildcard *.cpp)

# Компиляция всех .cpp файлов в build/task-1.out
all: always $(BUILD_DIR)/task-1.out

$(BUILD_DIR)/task-1.out:
	$(CC) $(CFLAGS) $(C_SRC) -o $@

# Всегда создавать директорию build/
always:
	mkdir -p $(BUILD_DIR)

# Очистка всех файлов сборки
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.o

# Не считать all, clean и always за имена файлов при вызове make
.PHONY: all clean always

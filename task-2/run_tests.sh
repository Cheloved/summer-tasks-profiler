#!/bin/bash

set -e

# Путь к исполняемому файлу
EXEC=./build/task-2.out

# Каталог для тестовых файлов
TEST_DIR=test_files
mkdir -p "$TEST_DIR"

# Очистка старых данных
rm -f "$TEST_DIR"/* actual_output.txt expected_output.txt

echo "=== Генерация тестовых файлов ==="

# 1. Файл с одним uint32_t (0x12345678)
printf '\x78\x56\x34\x12' > "$TEST_DIR/single.bin"

# 2. Файл с нулями (краевой случай)
printf '\x00\x00\x00\x00\x00\x00\x00\x00' > "$TEST_DIR/zeros.bin"

# 3. Файл с последовательностью чисел
for i in {0..9}; do
    printf "%04x%04x" $((i * 0x10)) $((i * 0x1000)) | xxd -r -p
done > "$TEST_DIR/sequence.bin"

# 4. Файл с чередующимися числами
for i in {0..9}; do
    printf '\x00\x00\x00\x01\x00\x00\x00\x00' # 1, 0
done > "$TEST_DIR/alternating.bin"

# 5. Файл с большим количеством данных
# head -c $((1024 * 1024 * 4)) /dev/urandom > "$TEST_DIR/bigfile.bin"

# 6. Файл с убывающей последовательностью
for i in {10..1}; do
    printf "%08x" $i | xxd -r -p
done > "$TEST_DIR/decreasing.bin"

# 7. Файл с максимальным uint32_t (0xFFFFFFFF)
printf '\xff\xff\xff\xff' > "$TEST_DIR/max_uint32.bin"

# 8. Файл с разрывами (нули как разделители)
for i in {1..5}; do
    printf "\x01\x00\x00\x00"
    printf "\x00\x00\x00\x00"
done > "$TEST_DIR/split_values.bin"

echo "=== Подготовка эталонных значений ==="

# Вспомогательная программа для вычисления ожидаемого хэша
cat > hash_file.py << 'EOF'
import sys
import mmap
import struct

def process_block(block, m_last_hash):
    hash_val = m_last_hash
    for i in range(len(block)):
        part_hash = 0
        prev_val = 0
        ind = 0
        for j in range(i, len(block)):
            val = block[j]
            if val == 0:
                break
            if prev_val > val:
                ind = 0
            part_hash ^= (val << ind)
            ind += 1
            prev_val = val
        hash_val ^= part_hash
    return hash_val

def hash_file(path):
    try:
        with open(path, "rb") as f:
            mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
            block_size = 0x100000  # 1MB
            m_last_hash = 0
            while True:
                data = mm.read(4 * block_size)
                if not data:
                    break
                block = list(struct.unpack('<%dI' % (len(data)//4), data))
                m_last_hash = process_block(block, m_last_hash)
            return m_last_hash
    except Exception as e:
        print(f"[ERROR] {path}: {e}")
        return 0

if __name__ == "__main__":
    import sys
    for path in sys.argv[1:]:
        h = hash_file(path)
        print(f"0x{h:08x}")
EOF

# Генерация ожидаемых хэшей по одному на файл
for file in "$TEST_DIR"/*.bin; do
    python3 hash_file.py "$file" >> expected_output.txt
done

echo "=== Сборка программы ==="
make clean && make

echo "=== Запуск тестов ==="
# Запуск программы по одному файлу за раз
for file in "$TEST_DIR"/*.bin; do
    echo -n "Тестируем файл: $file ... "
    "$EXEC" "$file" >> actual_output.txt
    echo "OK"
done

echo "=== Сравнение результатов ==="
if diff -w expected_output.txt actual_output.txt; then
    echo "Все тесты пройдены!"
else
    echo "Некоторые тесты провалены."
    echo "--- Ожидаемый вывод:"
    cat expected_output.txt
    echo "--- Полученный вывод:"
    cat actual_output.txt
fi

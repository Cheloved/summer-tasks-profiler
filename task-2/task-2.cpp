#include <vector>
#include <sys/mman.h>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "io.hpp"
#include "process.hpp"

/**
 * @brief Режим отладки: выводит промежуточные значения хэша при включённом DEBUG.
 *
 * Установите значение 1 для включения отладочной печати, 0 — для отключения.
 */
#define DEBUG 1

/**
 * @brief Вычисляет хэш файла по указанному пути.
 *
 * Функция открывает файл на чтение, считывает его содержимое блоками,
 * обрабатывает каждый блок с помощью класса data_processor_t и сохраняет
 * результат хэширования в переданной переменной.
 *
 * @param path   Путь к файлу, для которого нужно вычислить хэш.
 * @param result Указатель на переменную, в которую будет записан финальный хэш.
 *
 * @return 0 при успешном выполнении,
 *         отрицательное число в случае ошибки:
 *         - -1: Ошибка открытия файла.
 *         - -2: Нулевой указатель на результат.
 *         - -3: Ошибка чтения из файла.
 */
int hash_file(char* path, uint32_t* result)
{
    int fd = open(path, O_RDONLY);
    if ( fd == -1 )
    {
        fprintf(stderr, " [E] Ошибка при открытии файла %s\n", path);
        return -1;
    }

    if ( !result )
    {
        fprintf(stderr, " [E] Некорректный указатель в hash_file()\n");
        return -2;
    }

    data_processor_t dp;
    std::vector<std::uint32_t> buffer(BLOCK_SIZE);

    int bytes_read = 0;
    *result = 0;

    while ( (bytes_read = read_block(fd, &buffer)) > 0 )
    {
        *result = dp.process_block(buffer);

        #if DEBUG==1
            printf(" [debug] intermediate hash: 0x%08x\n", *result);
        #endif
    }

    if ( bytes_read < 0 )
        return -3;

    return 0;
}

/**
 * @brief Точка входа программы.
 *
 * Программа принимает один аргумент командной строки — путь к файлу.
 * Вычисляет хэш файла и выводит его в шестнадцатеричном виде.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 *             Первый аргумент должен быть путём к файлу.
 *
 * @return 0 при успешном завершении,
 *         положительное число в случае ошибки.
 */
int main(int argc, char** argv)
{
    if ( argc < 2 )
    {
        fprintf(stderr, " [E] В качестве аргумента должны быть пути к файлам\n");
        return 1;
    }

    // Массив для хранения PID дочерних процессов
    // для ожидания после запуска
    pid_t* pids = (pid_t*)calloc(argc-1, sizeof(pid_t));
    if ( !pids )
    {
        fprintf(stderr, " [E] Ошибка при выделении памяти\n");
        return 2;
    }

    // shared-memory массив для хранения результатов
    uint32_t* shared_hashes = (uint32_t*)mmap(NULL, (argc-1)*sizeof(uint32_t),
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED | MAP_ANONYMOUS,
                                         -1, 0);

    // Создание процессов под каждый аргумент
    for ( int i = 1; i < argc; i++ )
    {
        pids[i-1] = fork();
        if ( pids[i-1] < 0 )
        {
            fprintf(stderr, " [E] Ошибка при fork()\n");
            return 3;
        }

        // Дочерний процесс вызывает функцию хэширования
        // и выходит
        if ( pids[i-1] == 0 )
        {
            uint32_t hash = 0;
            int ret_code = hash_file(argv[i], &hash);
            if ( ret_code == 0 )
                shared_hashes[i-1] = hash;

            exit(ret_code);
        }
    }

    // Ожидание завершения процессов
    // и объединение
    uint32_t hash = 0;
    for ( int i = 0; i < argc-1; i++ )
    {
        int status;
        waitpid(pids[i], &status, 0);

        if ( status != 0 )
        {
            fprintf(stderr, " [E] Ошибка при обработке одного из файлов\n");
            return 4;
        }

        if ( i == 0 )
        {
            hash = shared_hashes[i];
            continue;
        }

        hash = hash ^ shared_hashes[i];
    }

    printf("0x%08x\n", hash);

    return 0;
}

#include "io.hpp"

int read_block(int fd, std::vector<std::uint32_t>* result)
{
    if ( fd == -1 )
    {
        fprintf(stderr, " [E] Несуществующий file descriptor в read_block()!\n");
        return -1;
    }

    if ( !result )
    {
        fprintf(stderr, " [E] Некорректный указатель в read_block()!\n");
        return -2;
    }

    ssize_t bytes_read = read(fd, result->data(), result->size() * sizeof(uint32_t));
    if ( bytes_read == -1 )
    {
        fprintf(stderr, " [E] Ошибка при чтении блока данных\n");
        return -3;
    }

    result->resize(bytes_read / sizeof(uint32_t));

    return bytes_read;
}

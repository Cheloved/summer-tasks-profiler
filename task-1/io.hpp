#pragma once

#include <vector>
#include <cstdint>
#include <unistd.h>
#include <stdio.h>

#define BLOCK_SIZE 0x100000

int read_block(int fd, std::vector<std::uint32_t>* result);

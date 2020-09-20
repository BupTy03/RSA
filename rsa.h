#pragma once

#include "utils.h"

#include <iosfwd>
#include <random>
#include <string>


struct rsa_keys
{
    big_int public_key;
    big_int private_key;
    big_int n;
};


rsa_keys generate_rsa_keys(std::mt19937& gen, std::size_t blockSize);
unsigned char* rsa(const unsigned char* first, const unsigned char* last,
                   unsigned char* dst, const big_int& key, const big_int& n);

void rsa(std::istream& input, std::ostream& output,
         std::size_t blockSize, const big_int& key, const big_int& n);
void rsa(const std::string& inputFilename, const std::string& outputFilename,
         std::size_t blockSize, const big_int& key, const big_int& n);

void rsa_mt(std::istream& input, std::ostream& output, std::size_t blockSize,
            const big_int& key, const big_int& n, std::size_t countThreads);
void rsa_mt(const std::string& inputFilename, const std::string& outputFilename, std::size_t blockSize,
            const big_int& key, const big_int& n, std::size_t countThreads);

void show_rsa_keys(std::size_t blockSize);
bool is_allowed_rsa_block_size(int blockSize);

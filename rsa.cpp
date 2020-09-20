#include "rsa.h"
#include "algorithms.h"
#include "RsaProcessor.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <array>


static constexpr std::array<int, 8> ALLOWED_RSA_BLOCK_SIZES = {
        32, 64, 128, 256, 512, 1024, 2048, 4096
};


rsa_keys generate_rsa_keys(std::mt19937& gen, std::size_t blockSize)
{
    rsa_keys result;

    const big_int p1 = big_random_prime(gen, blockSize);
    const big_int p2 = big_random_prime(gen, blockSize);
    const big_int euler = (p1 - 1) * (p2 - 1);

    result.n = p1 * p2;
    result.public_key = big_random_coprime(euler);
    result.private_key = multiplicative_inverse(result.public_key, euler);

    return result;
}

unsigned char* rsa(const unsigned char* first, const unsigned char* last,
                   unsigned char* dst, const big_int& key, const big_int& n)
{
    big_int chunk;
    boost::multiprecision::import_bits(chunk, first, last);
    const big_int result = power(chunk, key, modulo_multiply<big_int>(n));
    return boost::multiprecision::export_bits(result, dst, 8);
}

void rsa(std::istream& input, std::ostream& output, std::size_t blockSize, const big_int& key, const big_int& n)
{
    const std::vector<unsigned char> zerosBuffer(blockSize, 0);
    std::vector<unsigned char> buffer(blockSize, 0);

    const std::size_t fileSize = size_of_file(input);
    while(input)
    {
        const std::size_t buffSize = std::min(buffer.size(), fileSize - input.tellg());
        if(buffSize == 0)
            return;

        input.read(reinterpret_cast<char*>(buffer.data()), buffSize);

        const auto bytesWritten = static_cast<std::size_t>(rsa(buffer.data(), buffer.data() + buffSize, buffer.data(), key, n) - buffer.data());
        output.write(reinterpret_cast<const char*>(zerosBuffer.data()), zerosBuffer.size() - bytesWritten);
        output.write(reinterpret_cast<const char*>(buffer.data()), bytesWritten);
    }
}

void rsa(const std::string& inputFilename, const std::string& outputFilename,
         std::size_t blockSize, const big_int& key, const big_int& n)
{
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    rsa(inputFile, outputFile, blockSize, key, n);
}

void rsa_mt(std::istream& input, std::ostream& output, std::size_t blockSize,
            const big_int& key, const big_int& n, std::size_t countThreads)
{
    assert(countThreads > 0);

    RsaProcessor processor(blockSize, countThreads, &key, &n);

    const auto fileSize = size_of_file(input);
    const auto countBigBuffers = fileSize / processor.bufferSize();

    for(std::size_t buffIdx = 0; buffIdx < countBigBuffers; ++buffIdx)
        processor.process(input, output, processor.bufferSize());

    const auto remainingBytes = fileSize - (countBigBuffers * processor.bufferSize());
    processor.process(input, output, remainingBytes);
}

void rsa_mt(const std::string& inputFilename, const std::string& outputFilename,
            std::size_t blockSize, const big_int& key, const big_int& n, std::size_t countThreads)
{
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    rsa_mt(inputFile, outputFile, blockSize, key, n, countThreads);
}

void show_rsa_keys(std::size_t blockSize)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    const auto[pub, prv, n] = generate_rsa_keys(gen, blockSize);
    std::cout << "public = " << std::hex << pub
              << "\nprivate = " << std::hex << prv
              << "\nn = " << std::hex << n << std::endl;
}

bool is_allowed_rsa_block_size(int blockSize)
{
    return std::find(std::begin(ALLOWED_RSA_BLOCK_SIZES),
                     std::end(ALLOWED_RSA_BLOCK_SIZES), blockSize) != std::end(ALLOWED_RSA_BLOCK_SIZES);
}

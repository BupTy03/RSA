#include "rsa.h"
#include "algorithms.h"
#include "scope_exit.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <thread>
#include <mutex>
#include <future>


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

unsigned char* rsa(const unsigned char* first, const unsigned char* last, unsigned char* dst, const big_int& key, const big_int& n)
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

void rsa(const std::string& inputFilename, const std::string& outputFilename, std::size_t blockSize, const big_int& key, const big_int& n)
{
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    rsa(inputFile, outputFile, blockSize, key, n);
}

struct ComputeChunk
{
    explicit ComputeChunk(unsigned char* beginBuffer, unsigned char* endBuffer, const big_int& key, const big_int& n)
        : beginBuffer_(beginBuffer)
        , endBuffer_(endBuffer)
        , pKey_(&key)
        , pN_(&n)
    {}

    void operator()()
    {
        auto endWritten = rsa(beginBuffer_, endBuffer_, beginBuffer_, *pKey_, *pN_);
        if(endWritten != endBuffer_)
        {
            std::rotate(beginBuffer_, endWritten, endBuffer_);
            std::fill_n(beginBuffer_, endBuffer_ - endWritten, 0);
        }
    }

private:
    unsigned char* beginBuffer_;
    unsigned char* endBuffer_;
    const big_int* pKey_;
    const big_int* pN_;
};

void rsa_chunk_mt(std::vector<unsigned char>& buffer, std::size_t countBytes, std::size_t blockSize, const big_int& key, const big_int& n, std::size_t countThreads)
{
    std::vector<std::future<void>> tasks;
    SCOPE_EXIT{ for(auto& t : tasks) t.get(); };

    tasks.reserve(countThreads);
    for(std::size_t taskIdx = 0; taskIdx < countThreads; ++taskIdx)
    {
        auto beginChunk = buffer.data() + blockSize * taskIdx;
        auto endChunk = beginChunk + blockSize;

        tasks.emplace_back(std::async(std::launch::async, ComputeChunk(beginChunk, endChunk, key, n)));
    }

    const auto remainBytes = countBytes % countThreads;
    if(remainBytes > 0)
        rsa(buffer.data() + (countBytes - remainBytes), buffer.data() + countBytes, buffer.data(), key, n);
}

void rsa_mt(std::istream& input, std::ostream& output, std::size_t blockSize, const big_int& key, const big_int& n, std::size_t countThreads)
{
    assert(countThreads > 0);

    std::vector<unsigned char> bigBuffer(blockSize * countThreads, 0);
    const auto fileSize = size_of_file(input);
    const auto countBigBuffers = fileSize / bigBuffer.size();

    for(std::size_t buffIdx = 0; buffIdx < countBigBuffers; ++buffIdx)
    {
        input.read(reinterpret_cast<char*>(bigBuffer.data()), bigBuffer.size());
        rsa_chunk_mt(bigBuffer, bigBuffer.size(), blockSize, key, n, countThreads);
        output.write(reinterpret_cast<char*>(bigBuffer.data()), bigBuffer.size());
    }

    const auto remainBytes = fileSize % bigBuffer.size();
    if(remainBytes > 0)
    {
        input.read(reinterpret_cast<char*>(bigBuffer.data()), remainBytes);
        rsa_chunk_mt(bigBuffer, remainBytes, blockSize, key, n, countThreads);
        output.write(reinterpret_cast<char*>(bigBuffer.data()), blockSize * (remainBytes / blockSize + 1));
    }
}

void rsa_mt(const std::string& inputFilename, const std::string& outputFilename, std::size_t blockSize, const big_int& key, const big_int& n, std::size_t countThreads)
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
    constexpr int allowed[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096 };
    return std::find(std::begin(allowed), std::end(allowed), blockSize) != std::end(allowed);
}

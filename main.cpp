#include "algorithms.h"

#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include <functional>
#include <iterator>
#include <sstream>
#include <cstring>
#include <random>
#include <limits>

#include <boost/multiprecision/random.hpp>
#include <boost/multiprecision/cpp_int.hpp>


using big_int = boost::multiprecision::cpp_int;

big_int big_random_prime(std::mt19937& gen, std::size_t blockSize)
{
    std::uniform_int_distribution<std::uint16_t> dist(0, std::numeric_limits<std::uint16_t>::max());

    const big_int minValue = (big_int(1) << (blockSize * 8 / 2)) + 1;
    const big_int maxValue = minValue + std::numeric_limits<std::uint16_t>::max();

    big_int result = minValue + dist(gen);
    if((result & 1) == 0) ++result;

    while(!is_prime<big_int, 100>(result, gen))
        result += 2;

    assert(result < maxValue);
    return result;
}

big_int calculate_public_key(const big_int& n)
{
    boost::random::uniform_int_distribution<big_int> ui(0, n - 1);
    boost::random::mt19937 gen;
    big_int result = ui(gen);
    while (stein_gcd(result, n) != 1)
        result = ui(gen);

    return result;
}

struct rsa_keys
{
    big_int public_key;
    big_int private_key;
    big_int n;
};

rsa_keys generate_rsa_keys(std::mt19937& gen, std::size_t blockSize)
{
    rsa_keys result;

    const big_int p1 = big_random_prime(gen, blockSize);
    const big_int p2 = big_random_prime(gen, blockSize);
    const big_int euler = (p1 - 1) * (p2 - 1);

    result.n = p1 * p2;
    result.public_key = calculate_public_key(euler);
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

std::size_t size_of_file(std::istream& input)
{
    input.seekg(0, std::ios_base::end);
    const auto result = input.tellg();
    input.seekg(0);
    input.clear();
    return result;
}

std::size_t size_of_file(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    return size_of_file(file);
}

void rsa_encrypt(std::istream& input, std::ostream& output, std::size_t blockSize, const big_int& key, const big_int& n)
{
    const std::vector<unsigned char> zerosBuffer(blockSize, 0);
    std::vector<unsigned char> inputBuffer(blockSize, 0);
    std::vector<unsigned char> outputBuffer(blockSize, 0);

    const std::size_t fileSize = size_of_file(input);
    while(input)
    {
        const std::size_t buffSize = std::min(inputBuffer.size(), fileSize - input.tellg());
        if(buffSize == 0)
            return;

        input.read(reinterpret_cast<char*>(inputBuffer.data()), buffSize);

        const auto bytesWritten = static_cast<std::size_t>(rsa(inputBuffer.data(), inputBuffer.data() + buffSize, outputBuffer.data(), key, n) - outputBuffer.data());
        if(bytesWritten < outputBuffer.size())
            output.write(reinterpret_cast<const char*>(zerosBuffer.data()), outputBuffer.size() - bytesWritten);
        output.write(reinterpret_cast<const char*>(outputBuffer.data()), bytesWritten);
    }
}

void rsa_decrypt(std::istream& input, std::ostream& output, std::size_t blockSize, const big_int& key, const big_int& n)
{
    const std::vector<unsigned char> zerosBuffer(blockSize, 0);
    std::vector<unsigned char> inputBuffer(blockSize, 0);
    std::vector<unsigned char> outputBuffer(blockSize, 0);

    const std::size_t fileSize = size_of_file(input);
    while(input)
    {
        const std::size_t buffSize = std::min(inputBuffer.size(), fileSize - input.tellg());
        if(buffSize == 0)
            return;

        input.read(reinterpret_cast<char*>(inputBuffer.data()), buffSize);

        const auto bytesWritten = static_cast<std::size_t>(rsa(inputBuffer.data(), inputBuffer.data() + buffSize, outputBuffer.data(), key, n) - outputBuffer.data());
        if(bytesWritten < outputBuffer.size())
            output.write(reinterpret_cast<const char*>(zerosBuffer.data()), outputBuffer.size() - bytesWritten);
        output.write(reinterpret_cast<const char*>(outputBuffer.data()), bytesWritten);
    }
}

void rsa_encrypt(const char* inputFilename, const char* outputFilename, std::size_t blockSize, const big_int& key, const big_int& n)
{
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    rsa_encrypt(inputFile, outputFile, blockSize, key, n);
}

void rsa_decrypt(const char* inputFilename, const char* outputFilename, std::size_t blockSize, const big_int& key, const big_int& n)
{
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    rsa_decrypt(inputFile, outputFile, blockSize, key, n);
}

big_int from_hex(const char* str)
{
    std::istringstream istr(str);
    big_int result;
    istr >> std::hex >> result;
    return result;
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

bool is_allowed_block_size(int blockSize)
{
    constexpr int allowed[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096};
    return std::find(std::begin(allowed), std::end(allowed), blockSize) != std::end(allowed);
}

void print_help()
{
    std::cout << "Command format to generate keys:\n"
                 "RSA -k <blocksize (must be 32, 64, 128, 256, 512, 1024, 2048 or 4096)>\n"
                 "Example: RSA -k 256\n\n"
                 "Command format to encrypt file:\n"
                 "RSA -e <blocksize> <inputFile> <outputFile> <key> <n>\n"
                 "Example: RSA -e file.txt encrypted_file.txt 3AF434353324124 12312424325435\n\n"
                 "Command format to decrypt file:\n"
                 "RSA -d <blocksize> <inputFile> <outputFile> <key> <n>\n"
                 "Example: RSA -d file.txt decrypted_file.txt FAFAFAF2342342333 12312424325435\n\n"
                  << std::endl;
}

bool is_help(const char* command)
{
    constexpr const char* variants[] = { "-h", "h", "-help", "help", "?", "/?" };
    for(auto variant : variants)
    {
        if(std::strcmp(command, variant) == 0)
            return true;
    }

    return false;
}


int main(int argc, char* argv[])
{
    if(argc == 2 && is_help(argv[1]))
    {
        print_help();
    }
    if(argc == 3 && std::strcmp(argv[1], "-k") == 0)
    {
        const auto blockSize = std::atoi(argv[2]);
        if(!is_allowed_block_size(blockSize))
        {
            std::cout << "Error: invalid block size" << std::endl;
            print_help();
            return -1;
        }

        show_rsa_keys(static_cast<std::size_t>(blockSize));
    }
    else if(argc == 7)
    {
        const char* mode = argv[1];
        const auto blockSize = std::atoi(argv[2]);
        if(!is_allowed_block_size(blockSize))
        {
            std::cerr << "Error: invalid block size" << std::endl;
            print_help();
            return -1;
        }

        const char* inputFilename = argv[3];
        const char* outputFilename = argv[4];
        const big_int key = from_hex(argv[5]);
        const big_int n = from_hex(argv[6]);

        const big_int powerOfBlockSize = big_int(1) << (blockSize * 8);
        assert(n > powerOfBlockSize);

        if(std::strcmp(mode, "-e") == 0)
        {
            rsa_encrypt(inputFilename, outputFilename, blockSize, key, n);
        }
        else if(std::strcmp(mode, "-d") == 0)
        {
            if(size_of_file(inputFilename) % blockSize != 0)
            {
                std::cerr << "Error: encrypted file size must be a multiple of the block size" << std::endl;
                return -2;
            }

            rsa_decrypt(inputFilename, outputFilename, blockSize, key, n);
        }
        else
        {
            std::cerr << "Error: invalid mode" << std::endl;
            print_help();
            return -3;
        }
    }
    else
    {
        std::cerr << "Error: invalid arguments" << std::endl;
        print_help();
        return -4;
    }
    return 0;
}

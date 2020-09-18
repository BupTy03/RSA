#include "algorithms.h"

#include <iostream>
#include <fstream>
#include <array>
#include <functional>
#include <iterator>
#include <sstream>
#include <cstring>

#include <boost/multiprecision/cpp_int.hpp>


using cpp_int = boost::multiprecision::cpp_int;


cpp_int big_random_prime(std::mt19937& gen)
{
    std::ifstream file("../very_big_primes.txt");
    file.unsetf(std::ios::skipws);

    const std::size_t countLines = [&]() {
        std::istream_iterator<char> first(file);
        std::istream_iterator<char> last;

        return std::count(first, last, '\n');
    }();

    std::uniform_int_distribution<std::size_t> distrib(0, countLines - 1);
    const std::size_t lineNumber = distrib(gen);

    file.clear();
    file.seekg(0);

    std::istream_iterator<char> first(file);
    std::istream_iterator<char> last;

    auto it = first;
    if(lineNumber > 0)
    {
        std::size_t currentLine = 0;
        for (; it != last; ++it)
        {
            if (*it == '\n')
            {
                ++currentLine;
                if(currentLine == lineNumber - 1)
                {
                    ++it;
                    break;
                }
            }
        }
    }

    std::string prime;
    for(; it != last; ++it)
    {
        char ch = *it;
        if(ch == '\n')
            break;

        prime += ch;
    }

    return cpp_int(prime);
}

cpp_int calculate_public_key(const cpp_int& n, std::mt19937& gen) { return big_random_prime(gen); }

struct rsa_keys
{
    cpp_int public_key;
    cpp_int private_key;
    cpp_int n;
};

rsa_keys generate_rsa_keys(std::mt19937& gen)
{
    rsa_keys result;

    const cpp_int p1 = big_random_prime(gen);
    const cpp_int p2 = big_random_prime(gen);
    const cpp_int euler = (p1 - 1) * (p2 - 1);

    result.n = p1 * p2;
    result.public_key = calculate_public_key(euler, gen);
    result.private_key = multiplicative_inverse(result.public_key, euler);

    return result;
}

unsigned char* rsa(const unsigned char* first, const unsigned char* last, unsigned char* dst, const cpp_int& key, const cpp_int& n)
{
    cpp_int chunk;
    boost::multiprecision::import_bits(chunk, first, last);
    const cpp_int result = power(chunk, key, modulo_multiply<cpp_int>(n));
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

void rsa(std::istream& input, std::ostream& output, const cpp_int& key, const cpp_int& n)
{
    const std::array<unsigned char, 512> zerosBuffer = {0};
    std::array<unsigned char, 512> inputBuffer = {0};
    std::array<unsigned char, 512> outputBuffer = {0};

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

cpp_int from_hex(const char* str)
{
    std::istringstream istr(str);
    cpp_int result;
    istr >> std::hex >> result;
    return result;
}

void show_rsa_keys()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    const auto[pub, prv, n] = generate_rsa_keys(gen);
    std::cout << "public = " << std::hex << pub
            << "\nprivate = " << std::hex << prv
            << "\nn = " << std::hex << n << std::endl;
}

void rsa(const char* inputFilename, const char* outputFilename, const cpp_int& key, const cpp_int& n)
{
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    rsa(inputFile, outputFile, key, n);
}


int main(int argc, char* argv[])
{
    if(argc == 2 && std::strcmp(argv[1], "-keys") == 0)
    {
        show_rsa_keys();
    }
    else if(argc == 5)
    {
        const char* inputFilename = argv[1];
        const char* outputFilename = argv[2];
        const cpp_int key = from_hex(argv[3]);
        const cpp_int n = from_hex(argv[4]);

        const cpp_int powerOfBlockSize = power(cpp_int(2), 4096, std::multiplies<cpp_int>());
        assert(n > powerOfBlockSize);

        rsa(inputFilename, outputFilename, key, n);
    }
    else
    {
        return -1;
    }

    return 0;
}

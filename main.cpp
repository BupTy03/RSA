#include "algorithms.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <iterator>


cpp_int big_random_prime(std::mt19937& gen)
{
//    const auto& primesList = big_primes();
//    std::uniform_int_distribution<std::size_t> distrib(0, primesList.size() - 1);
//    return primesList.at(distrib(gen));

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

    std::cout << prime << std::endl;
    return cpp_int(prime);
}

cpp_int calculate_public_key(const cpp_int& n, std::mt19937& gen)
{
    std::uniform_int_distribution<std::uint64_t> distrib(65537, std::min(std::uint64_t{n - 1}, std::numeric_limits<std::uint64_t>::max()));
    cpp_int result = distrib(gen);
    while(stein_gcd(result, n) != 1)
        result = distrib(gen);

    return result;
}

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

std::vector<cpp_int> rsa(const std::vector<cpp_int>& data, cpp_int key, cpp_int n)
{
    std::vector<cpp_int> result;
    result.reserve(data.size());
    for(const auto& chunk : data)
        result.emplace_back(power(chunk, key, modulo_multiply<cpp_int>(n)));

    return result;
}

void print(const std::vector<cpp_int>& vec)
{
    std::cout << "[ ";
    for(const auto& ch : vec)
        std::cout << ch << " ";
    std::cout << ']' << std::endl;
}


int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    const auto[pub, prv, n] = generate_rsa_keys(gen);
    std::cout << "pub = " << pub << ", prv = " << prv << ", n = " << n << std::endl;

    const std::vector<cpp_int> data = {1, 2, 3, 4};

    const auto encrypted = rsa(data, pub, n);
    print(encrypted);

    const auto decrypted = rsa(encrypted, prv, n);
    print(decrypted);

    return 0;
}

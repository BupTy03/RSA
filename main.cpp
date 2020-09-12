#include "algorithms.h"

#include <iostream>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>


using cpp_int = boost::multiprecision::cpp_int;


struct rsa_keys
{
    cpp_int public_key;
    cpp_int private_key;
    cpp_int n;
};

cpp_int calculate_public_key(const cpp_int& n, std::mt19937& gen)
{
    std::uniform_int_distribution<std::uint64_t> distrib(65537, std::min(std::uint64_t{n - 1}, std::numeric_limits<std::uint64_t>::max()));
    cpp_int result = distrib(gen);
    while(stein_gcd(result, n) != 1)
        result = distrib(gen);

    return result;
}

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

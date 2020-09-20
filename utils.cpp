#include "utils.h"
#include "algorithms.h"

#include <boost/multiprecision/random.hpp>

#include <sstream>
#include <istream>


big_int big_random_prime(std::mt19937& gen, std::size_t blockSize)
{
    std::uniform_int_distribution<std::uint16_t> dist(0, std::numeric_limits<std::uint16_t>::max());

    const big_int minValue = (big_int(1) << (blockSize * 8 / 2)) + 1;
    const big_int maxValue = minValue + std::numeric_limits<std::uint16_t>::max();

    big_int result = minValue + dist(gen);
    if((result & 1) == 0) ++result;

    while(!is_prime<big_int, 100>(result, gen))
        result += 2;

    return result;
}

big_int big_random_coprime(const big_int& n)
{
    boost::random::uniform_int_distribution<big_int> ui(0, n - 1);
    boost::random::mt19937 gen;
    big_int result = ui(gen);
    while (stein_gcd(result, n) != 1)
        result = ui(gen);

    return result;
}

big_int from_hex(const std::string& str)
{
    std::istringstream istr(str);
    big_int result;
    istr >> std::hex >> result;
    return result;
}

std::size_t size_of_file(std::istream& input)
{
    input.seekg(0, std::ios_base::end);
    const auto result = input.tellg();
    input.seekg(0);
    input.clear();
    return result;
}

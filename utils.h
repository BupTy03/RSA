#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <random>
#include <iosfwd>
#include <cstdint>


using big_int = boost::multiprecision::cpp_int;

big_int big_random_prime(std::mt19937& gen, std::size_t blockSize);
big_int big_random_coprime(const big_int& n);

big_int from_hex(const std::string& str);
std::size_t size_of_file(std::istream& input);
std::size_t size_of_file(const std::string& filename);

#pragma once

#include <functional>
#include <random>
#include <cassert>
#include <limits>
#include <array>
#include <iostream>


template<typename I>
struct modulo_multiply
{
    explicit modulo_multiply(const I& i) : modulus_{i} {}
    I modulus() const { return modulus; }
    I operator()(const I& n, const I& m) const {
        return (n * m) % modulus_;
    }

private:
    const I modulus_;
};

template<typename T> T identity_element(std::plus<T>) { return T{0}; }
template<typename T> T identity_element(std::multiplies<T>) { return T{1}; }
template<typename T> bool identity_element(std::logical_and<T>) { return true; }
template<typename T> bool identity_element(std::logical_or<T>) { return false; }
template<typename I> I identity_element(const modulo_multiply<I>&) { return I{1}; }

template<typename T, typename N, typename BinaryOperation>
T power(T x, N n, BinaryOperation op)
{
    assert(n >= 0);
    if (n == 0) return identity_element(op);

    while ((n & 1) == 0)
    {
        n >>= 1;
        x = op(x, x);
    }

    T result = x;
    n >>= 1;
    while (n != 0)
    {
        x = op(x, x);
        if ((n & 1) != 0)
            result = op(result, x);

        n >>= 1;
    }

    return result;
}

template<typename I>
bool miller_rabin_test(I n, I q, I k, I w)
{
    // assert(n > 1 && n - 1 == power(I{2}, k, std::multiplies<>()) * q && odd(q));
    modulo_multiply<I> mmult{n};
    I x = power(w, q, mmult);
    if(x == I{1} || x == n - I{1})
        return true;

    for(I i{1}; i < k; ++i)
    {
        // invariant: x = w**(2**(i - 1)q)
        x = mmult(x, x);
        if(x == n - I{1}) return true;
        if(x == I{1}) return false;
    }

    return false;
}

template<typename I>
std::pair<I, I> find_q_and_k(I n)
{
    I k{0};
    while ((n & 1) == I{0})
    {
        n >>= 1;
        ++k;
    }

    return {n, k};
}

template<typename I>
bool is_prime(I n, std::mt19937& gen)
{
    assert(n > 3);
    assert((n & 1) == 1);

    constexpr std::size_t countChecks = 100;

    std::uniform_int_distribution<I> distrib(3, n - 1);
    const auto[q, k] = find_q_and_k(n - 1);
    for(std::size_t i = 0; i < countChecks; ++i)
    {
        const auto w = distrib(gen);
        if(!miller_rabin_test(n, q, k, w))
            return false;
    }

    return true;
}

const auto& bigPrimes()
{
    static const auto primes = []()
    {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uint64_t p = 999999999;
        std::array<std::uint64_t, 10000> result;

        std::size_t i = 0;
        while(i < result.size())
        {
            if(is_prime(p, gen))
            {
                result[i] = p;
                ++i;
            }

            p += 2;
        }

        return result;
    }();

    return primes;
}

std::uint64_t big_random_prime(std::mt19937& gen)
{
    const auto& primesList = bigPrimes();
    std::uniform_int_distribution<std::uint64_t> distrib(0, primesList.size() - 1);
    return primesList.at(distrib(gen));
}

template<typename N> std::pair<N, N> quotient_remainder(const N& a, const N& b) { return {a / b, a % b}; }

template<typename E>
std::pair<E, E> extended_gcd(E a, E b)
{
    E x0{1};
    E x1{0};

    while (b != E{0})
    {
        std::pair<E, E> qr = quotient_remainder(a, b);
        E x2 = x0 - qr.first * x1;
        x0 = x1;
        x1 = x2;
        a = b;
        b = qr.second;
    }

    return {x0, a};
}

template<typename N>
N stein_gcd(N m, N n)
{
    using std::swap;

    if(n < N{0}) m = -m;
    if(n < N{0}) n = -n;
    if(m == N{0}) return n;
    if(n == N{0}) return m;

    // assert(m > 0 && n > 0);
    int d_m = 0;
    while ((m & 1) == 0) { m >>= 1; ++d_m; }

    int d_n = 0;
    while ((n & 1) == 0) { n >>= 1; ++d_n; }

    // assert(odd(m) && odd(n));
    while (m != n)
    {
        if(n > m)
            swap(n, m);

        m-=n;
        do m >>= 1; while ((m & 1) == 0);
    }

    // assert(m == n);
    return m << std::min(d_m, d_n);
}

template<typename I>
I multiplicative_inverse(I a, I n)
{
    std::pair<I, I> p = extended_gcd(a, n);

    std::cout << "first: " << p.first << ", second: " << p.second << std::endl;

    if(p.second != I{1}) return I{0};
    if(p.first < I{0}) return p.first + n;
    return p.first;
}

#pragma once

#include <functional>
#include <random>
#include <cassert>
#include <limits>
#include <array>


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
bool miller_rabin_test(const I& n, const I& q, const I& k, const I& w)
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
    I k(0);
    while ((n & 1) == 0)
    {
        n >>= 1;
        ++k;
    }

    return std::pair<I, I>(n, k);
}

template<typename I, std::size_t CountChecks>
bool is_prime(const I& n, std::mt19937& gen)
{
    assert(n > 2);
    assert((n & 1) == 1);

    if(n % 5 == 0) return false;
    if(n % 7 == 0) return false;
    if(n % 11 == 0) return false;
    if(n % 13 == 0) return false;
    if(n % 17 == 0) return false;
    if(n % 19 == 0) return false;
    if(n % 23 == 0) return false;
    if(n % 29 == 0) return false;
    if(n % 31 == 0) return false;
    if(n % 41 == 0) return false;
    if(n % 43 == 0) return false;
    if(n % 47 == 0) return false;
    if(n % 53 == 0) return false;
    if(n % 59 == 0) return false;
    if(n % 61 == 0) return false;
    if(n % 67 == 0) return false;
    if(n % 71 == 0) return false;
    if(n % 73 == 0) return false;
    if(n % 79 == 0) return false;
    if(n % 83 == 0) return false;
    if(n % 89 == 0) return false;
    if(n % 97 == 0) return false;
    if(n % 101 == 0) return false;
    if(n % 103 == 0) return false;
    if(n % 107 == 0) return false;
    if(n % 109 == 0) return false;
    if(n % 113 == 0) return false;
    if(n % 127 == 0) return false;
    if(n % 131 == 0) return false;
    if(n % 137 == 0) return false;
    if(n % 139 == 0) return false;
    if(n % 149 == 0) return false;
    if(n % 151 == 0) return false;
    if(n % 163 == 0) return false;
    if(n % 167 == 0) return false;
    if(n % 173 == 0) return false;
    if(n % 179 == 0) return false;
    if(n % 181 == 0) return false;
    if(n % 191 == 0) return false;
    if(n % 193 == 0) return false;
    if(n % 197 == 0) return false;
    if(n % 199 == 0) return false;
    if(n % 211 == 0) return false;
    if(n % 223 == 0) return false;
    if(n % 227 == 0) return false;
    if(n % 229 == 0) return false;
    if(n % 233 == 0) return false;
    if(n % 239 == 0) return false;
    if(n % 241 == 0) return false;
    if(n % 251 == 0) return false;

    const I maxN = n - 1;
    const std::uint64_t maxVal = maxN > std::numeric_limits<std::uint64_t>::max() ? std::numeric_limits<std::uint64_t>::max() : std::uint64_t{maxN};
    std::uniform_int_distribution<std::uint64_t> distrib(3, maxVal);
    const std::pair<I, I> q_k = find_q_and_k(maxN);
    for(std::size_t i = 0; i < CountChecks; ++i)
    {
        const I w = distrib(gen);
        if(!miller_rabin_test(I(n), I(q_k.first), I(q_k.second), I(w)))
            return false;
    }

    return true;
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

    assert(m >= N{0});
    assert(n >= N{0});
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
I multiplicative_inverse(const I& a, const I& n)
{
    std::pair<I, I> p = extended_gcd(a, n);

    if(p.second != I{1}) return I{0};
    if(p.first < I{0}) return p.first + n;
    return p.first;
}

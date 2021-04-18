#pragma once
#include <random>
#include <cstdint>

class Random
{
    Random() = delete;

    static std::random_device rd;
    static uint64_t Seed[2];


    static uint64_t rotl(const uint64_t x, int32_t k)
    {
        return (x << k) | (x >> (64 - k));
    }

    static uint64_t next()
    {
        const uint64_t s0 = Seed[0];
        uint64_t s1 = Seed[1];
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        Seed[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
        Seed[1] = rotl(s1, 37); // c

        return result;
    }

public:
    static double RDouble(const double Min = 0.0, const double Max = 1.0)
    {
        const double Rand = next() * (1.0 / static_cast<double>(std::numeric_limits<uint64_t>::max()));
        return Min + (Max - Min) * Rand;
    }
};

inline std::random_device Random::rd{};
inline uint64_t Random::Seed[2] = {rd(), rd()};
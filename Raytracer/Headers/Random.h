#pragma once
#include <random>


inline double RandomD(const double Min = 0.0, const double Max = 1.0)
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_real_distribution<double> R(0.0, 1.0);

    const double Rand = R(gen);
    return Min + (Max - Min) * Rand;
}
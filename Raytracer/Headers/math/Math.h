#pragma once
#include "Vector.h"
#include "../Random.h"




template<class T>
constexpr T Clamp(T X, T Low, T High)
{
    return std::min(std::max(X, Low), High);
}

constexpr double DegToRad(const double& Degrees)
{
    return PI / 180.0 * Degrees;
}
constexpr double RadToDeg(const double& Radians)
{
    return 180.0 * Radians / PI;
}

inline double LInterp(const double A, const double B, const double Alpha)
{
    return A + (B - A) * Alpha;
}
inline Vector3 LInterp(const Vector3& A, const Vector3& B, const double Alpha)
{
    return A + (B - A) * Alpha;
}

inline double GetConeAngleByFittingSphere(const double SphereRadius, const double Distance)
{
    return std::asin(SphereRadius / Distance);
}

inline Vector3 GetRandomUnitVectorInsideCone(const Vector3& ConeDir, const double ConeAngle)
{
    const Vector3 U = ConeDir.MinAxis();
    const Vector3 V = U ^ ConeDir;

    const double Phi = RandomD(-PI, PI);
    const double Theta = RandomD(0.0, ConeAngle);

    const Vector3 Result = std::sin(Theta) * (std::cos(Phi) * U + std::sin(Phi) * V) + std::cos(Theta) * ConeDir;
    return Result.Normalized();
}

inline Vector3 GetRandomVectorHemisphere(const Vector3& Normal)
{
    return GetRandomUnitVectorInsideCone(Normal, PI);
}

inline Vector3 Refract(const Vector3& I, const Vector3& N, const double Index)
{
    double cosI = -Clamp(N | I, -1.0, 1.0);
    double etaI = 1.0, etaT = Index;
    Vector3 Normal = N;

    if (cosI < 0.0)
    {
        cosI = -cosI;
    }
    else
    {
        std::swap(etaI, etaT);
        Normal = -Normal;
    }
    const double eta = etaI / etaT;
    const double K = 1.0 - eta * eta * (1.0 - cosI * cosI);
    return K < 0 ? Vector3(0.0) : eta * I + (eta * cosI - std::sqrt(K)) * Normal;
}



inline Vector2 CartesianToPolar(const Vector2& Cartesian)
{
    return Vector2(std::sqrt(Cartesian.X * Cartesian.X + Cartesian.Y * Cartesian.Y), std::atan2(Cartesian.Y, Cartesian.X));
}
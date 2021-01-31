#pragma once
#include <iostream>
#include <random>
#include <vector>

#include "Scene.h"
#include "math/mvector.h"


enum class EColorChannels : uint8_t
{
	R = 1,
	G = 2,
	B = 4,
	A = 8
};

constexpr EColorChannels operator|(const EColorChannels A, const EColorChannels B)
{
    return static_cast<EColorChannels>(static_cast<int>(A) | static_cast<int>(B));
}
constexpr EColorChannels operator&(const EColorChannels A, const EColorChannels B)
{
    return static_cast<EColorChannels>(static_cast<int>(A) & static_cast<int>(B));
}
constexpr EColorChannels operator~(const EColorChannels A)
{
    return static_cast<EColorChannels>(~static_cast<int>(A));
}



template<class T>
constexpr T Clamp(T X, T Low, T High)
{
    return std::min(std::max(X, Low), High);
}

inline double LInterp(const double A, const double B, const double Alpha)
{
    return A + (B - A) * Alpha;
}
inline Vector3 LInterp(const Vector3& A, const Vector3& B, const double Alpha)
{
    return A + (B - A) * Alpha;
}

inline double RandomD(const double Min = 0.0, const double Max = 1.0)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> R(0.0, 1.0);

    const double X = R(gen);
    return Min + X * (Max - Min);
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

inline Vector3 Refract(const Vector3& I, const Vector3& N, const double Index)
{
    double cosi = -Clamp(N | I, -1.0, 1.0);
    double etai = 1.0, etat = Index;
    Vector3 Normal = N;

    if (cosi < 0.0) /* Ray is inside the object */
    {
        cosi = -cosi;
        std::swap(etai, etat);
        Normal = -Normal;
    }
    double eta = etai / etat;
    double k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    return k < 0.0 ? Vector3(0.0) : I * eta + Normal * (eta * cosi - sqrt(k));
}

inline void DrawPercent(const uint32_t Current, const uint32_t Max, const uint8_t Step, const char* Task = "Processing")
{
    const uint8_t Denom = 100 / Step;
    if (Current % (Max / Denom) == 0)
    {
        system(("CLS"));
        std::cout << Task << "... " << Current * 100 / Max << "%\n";
    }
}
inline void DrawTask(const char* Task = "Processing")
{
    system(("CLS"));
    std::cout << Task << "... ";
}

inline Texture3D Texture3DFrom1D(const Texture1D& Texture, const EColorChannels Channels)
{
    Texture3D ResultTexture(Texture.GetHeight(), Texture.GetWidth());

    for (uint32_t i = 0; i < Texture.GetHeight(); i++)
        for (uint32_t j = 0; j < Texture.GetWidth(); j++)
		{  	
	        Vector3 V;
	        if ((Channels & EColorChannels::R) == EColorChannels::R)
	            V.X = Texture.Get(j, i);
	        if ((Channels & EColorChannels::G) == EColorChannels::G)
	            V.Y = Texture.Get(j, i);
	        if ((Channels & EColorChannels::B) == EColorChannels::B)
	            V.Z = Texture.Get(j, i);
	        ResultTexture.Write(V, j, i);
		}
	
    return ResultTexture;
}

namespace BRDF
{
    namespace
    {
        double G1(const double Dot, const double K)
        {
            return Dot / (Dot * (1.0 - K) + K);
        }

        double G(const double NdotL, const double NdotV, const double Roughness)
        {
            const double K = (Roughness + 1.0) * (Roughness + 1.0) / 8.0;
            return G1(NdotL, K) * G1(NdotV, K);
        }

        double D(const double Roughness, const double HdotN)
        {
            const double R = std::pow(Roughness, 4);
            const double D = PI * std::pow(HdotN * HdotN * (R - 1.0) + 1.0, 2);
            return R / D;
        }

        double F(double HdotV)
        {
            const double F0 = std::pow((1.0 - 2.0) / (1.0 + 2.0), 2);
            return F0 + (1.0 - F0) * std::pow(1.0 - HdotV, 5.0);
        }
    }

    static double BRDF(const Vector3 N, const Vector3 V, const Vector3 L, double Roughness, double& Fresnel)
    {
        const Vector3 H = (L + V).Normalized();
        Roughness = Clamp(Roughness, 1e-2, 1.0);
        const double HdotN = std::max(H | N, 0.0);
        const double VdotN = std::max(V | N, 0.0);
        const double LdotN = std::max(L | N, 0.0);
        const double HdotV = std::max(V | H, 0.0);
        Fresnel = F(HdotV);
        return Fresnel * G(LdotN, VdotN, Roughness) * D(Roughness, HdotN) / std::max(VdotN * LdotN * 4.0, 1e-4);
    }
}
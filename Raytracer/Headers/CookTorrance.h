#pragma once

#include "ShadingModel.h"
#include "Scene.h"
#include "Material.h"
#include "Core.h"


class CookTorrance : public BRDF
{
public:
	CookTorrance() {}

	virtual BRDFResult Get(const RLightInfo& LightInfo) const override;
    virtual double GetPDF(const RLightInfo& LightInfo) const override;
    virtual Vector3 Sample(const RLightInfo& LightInfo) const override;

private:
	virtual RMaterial GetDefaultMaterial() const override;

    double G1(const double Dot, const double K) const;

    double G(const double NdotL, const double NdotV, const double Roughness) const;

    double D(const double Roughness, const double HdotN) const;

	Vector3 F(double HdotV, const Vector3& F0) const;
};

inline BRDFResult CookTorrance::Get(const RLightInfo& LightInfo) const
{
    const Vector3 N = LightInfo.Normal;
    const Vector3 L = LightInfo.Light;
    const Vector3 V = LightInfo.View;

    //if ((V | N) < 0.0 || (L | N) < 0.0) return BRDFResult(Vector3(0.0), Vector3(0.0));

    double Roughness = 0.5;
    LightInfo.Mat->GetFloatProperty(Roughness, "Roughness");

    double IoR = 1.0;
    LightInfo.Mat->GetFloatProperty(IoR, "RefractionIndex");

    double Metallic = 0.0;
    LightInfo.Mat->GetFloatProperty(Metallic, "Metallic");

    Vector3 Color = { 0.0, 0.0, 0.0 };
    LightInfo.Mat->GetVectorProperty(Color, "Color");


    const Vector3 H = (L + V).Normalized();
    Roughness = Clamp(Roughness, 0.001, 1.0);
    const double Alpha2 = std::pow(Roughness, 4);
    const double HdotN = std::max(H | N, 0.0);
    const double VdotN = std::max(V | N, 0.0);
    const double LdotN = std::max(L | N, 0.0);
    const double HdotV = std::max(V | H, 0.0);

    Vector3 F0 = Vector3(std::abs((1.0 - IoR) / (1.0 + IoR)));
    F0 = F0 * F0;
    F0 = LInterp(F0, Color, Metallic);
    const Vector3 Fresnel = F(HdotV, F0);

     
    const Vector3 Rs = Fresnel * G(LdotN, VdotN, Alpha2) * D(Alpha2, HdotN) / std::max(VdotN * LdotN * 4.0, 1e-6);
    const Vector3 Kd = (Vector3(1.0) - Fresnel) * (1.0 - Metallic);

    const Vector3 Diffuse = Color * Kd / PI;
    const Vector3 Specular = Rs;

    return BRDFResult(Diffuse + Specular, Fresnel, Diffuse, Specular);
}

inline double CookTorrance::GetPDF(const RLightInfo& LightInfo) const
{
    double Roughness = 1.0;
    LightInfo.Mat->GetFloatProperty(Roughness, "Roughness");
    Roughness = Clamp(Roughness, 0.001, 1.0);

    const Vector3 H = (LightInfo.Light + LightInfo.View).Normalized();
    const double NoH = LightInfo.Normal | H;
    const double Alpha = Roughness * Roughness; 
    const double Alpha2 = Alpha * Alpha;

    return D(Alpha2, NoH) * (NoH) / (4.0 * (LightInfo.View | H));
}

inline Vector3 CookTorrance::Sample(const RLightInfo& LightInfo) const
{
    double Roughness = 0.5;
    LightInfo.Mat->GetFloatProperty(Roughness, "Roughness");
    const double Alpha2 = Roughness * Roughness * Roughness * Roughness;

    const double Rand = Random::RDouble();
    const double Theta = std::acos(std::sqrt((1.0 - Rand) / ((Alpha2 - 1.0) * Rand + 1.0)));
    const double Phi = Random::RDouble() * 2.0 * PI;

    const Vector3 LocalSample = SphericalToUnitCartesian(Vector2(Theta, Phi));

    return TransformToWorld(LocalSample, LightInfo.Normal).Normalized();
}

inline RMaterial CookTorrance::GetDefaultMaterial() const
{
	return RMaterial();
}

inline double CookTorrance::G1(const double Dot, const double Alpha2) const
{
    const double N = 2.0 * Dot;
    const double D = Dot + std::sqrt(Alpha2 + (1.0 - Alpha2) * Dot * Dot);
    return N / D;
}

inline double CookTorrance::G(const double NdotL, const double NdotV, const double Alpha2) const
{
    return G1(NdotL, Alpha2) * G1(NdotV, Alpha2);
}

inline double CookTorrance::D(const double Alpha2, const double HdotN) const
{
    const double D = PI * std::pow(HdotN * HdotN * (Alpha2 - 1.0) + 1.0, 2);
    return Alpha2 / D;
}

inline Vector3 CookTorrance::F(const double HdotV, const Vector3& F0) const
{
    return F0 + (Vector3(1.0) - F0) * std::pow(1.0 - HdotV, 5.0);
}


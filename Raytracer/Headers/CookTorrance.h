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

private:
	virtual class RMaterialPBR* GetDefaultMaterial() const override;

    double G1(const double Dot, const double K) const;

    double G(const double NdotL, const double NdotV, const double Roughness) const;

    double D(const double Roughness, const double HdotN) const;

	double F(double HdotV) const;
};

inline BRDFResult CookTorrance::Get(const RLightInfo& LightInfo) const
{
    const Vector3 N = LightInfo.Hit.Normal;
    const Vector3 L = LightInfo.GetLightVector();
    const Vector3 V = LightInfo.View;

    const double Distance = LightInfo.GetLightDistance();
    const double Attenuation = Distance * Distance;
    const Vector3 Radiance = LightInfo.GetLightColor() / Attenuation;

    auto PBRMat = dynamic_cast<RMaterialPBR*>(LightInfo.Hit.Mat);
    if (!PBRMat) PBRMat = GetDefaultMaterial();
    double Roughness = PBRMat->Roughness;

    const Vector3 H = (L + V).Normalized();
    Roughness = Clamp(Roughness, 1e-2, 1.0);
    const double HdotN = std::max(H | N, 0.0);
    const double VdotN = std::max(V | N, 0.0);
    const double LdotN = std::max(L | N, 0.0);
    const double HdotV = std::max(V | H, 0.0);
    const double Fresnel = F(HdotV);

     
    const double Rs = Fresnel * G(LdotN, VdotN, Roughness) * D(Roughness, HdotN) / std::max(PI * VdotN * LdotN * 4.0, 1e-4);
    const double Kd = LdotN * (1.0 - Fresnel) * (1.0 - PBRMat->Metallic) / PI;

    const Vector3 OutColor = (PBRMat->Color * Kd + Vector3(Rs)) * Radiance;

    return BRDFResult(OutColor, Fresnel);
}

inline RMaterialPBR* CookTorrance::GetDefaultMaterial() const
{
	return new RMaterialPBR(Vector3(1.0, 1.0, 0.5), 0.5, 0.0, 1.0, 0.0, Vector3(0.0));
}

inline double CookTorrance::G1(const double Dot, const double K) const
{
    return Dot / (Dot * (1.0 - K) + K);
}

inline double CookTorrance::G(const double NdotL, const double NdotV, const double Roughness) const
{
    const double K = (Roughness + 1.0) * (Roughness + 1.0) / 8.0;
    return G1(NdotL, K) * G1(NdotV, K);
}

inline double CookTorrance::D(const double Roughness, const double HdotN) const
{
    const double R = std::pow(Roughness, 4);
    const double D = PI * std::pow(HdotN * HdotN * (R - 1.0) + 1.0, 2);
    return R / D;
}

inline double CookTorrance::F(double HdotV) const
{
    const double F0 = std::pow((1.0 - 2.0) / (1.0 + 2.0), 2);
    return F0 + (1.0 - F0) * std::pow(1.0 - HdotV, 5.0);
}


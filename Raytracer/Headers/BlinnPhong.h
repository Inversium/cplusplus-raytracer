#pragma once

#include "ShadingModel.h"
#include "Scene.h"
#include "Material.h"
#include "Core.h"


class BlinnPhong : public BRDF
{

public:
	BlinnPhong() {}

	virtual BRDFResult Get(const RLightInfo& LightInfo) const override;

private:
	virtual RMaterial GetDefaultMaterial() const override;

};

inline RMaterial BlinnPhong::GetDefaultMaterial() const
{
	return RMaterial();
}


inline BRDFResult BlinnPhong::Get(const RLightInfo& LightInfo) const
{
	auto Mat = LightInfo.Mat;

	double SpecularExponent = 5.0;
	Mat->GetFloatProperty(SpecularExponent, "SpecularExponent");

	Vector3 Color = { 0.0, 0.0, 0.0 };
	Mat->GetVectorProperty(Color, "Color");

	const auto L = LightInfo.Light;

	/* Diffuse lighting */
	const double Lambertian = std::max(L | LightInfo.Normal, 0.0);

	/* Specular highlights */
	double Specular = 0.0;
	if (Lambertian > 0.0)
	{
		const auto H = (L + LightInfo.View).Normalized();
		const double SpecularAngle = H | LightInfo.Normal;
		Specular = std::pow(SpecularAngle, SpecularExponent);
	}

	Vector3 Diffuse = Color * Lambertian / PI;

	
	return BRDFResult(Diffuse + Vector3(Specular), Vector3(0.04), Diffuse, Vector3(Specular));
}
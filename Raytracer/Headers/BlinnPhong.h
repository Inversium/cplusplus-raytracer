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
	virtual class RMaterialBlinnPhong* GetDefaultMaterial() const override;

};

inline RMaterialBlinnPhong* BlinnPhong::GetDefaultMaterial() const
{
	return new RMaterialBlinnPhong(Vector3(0.0), 1.0);
}


inline BRDFResult BlinnPhong::Get(const RLightInfo& LightInfo) const
{
	auto Mat = dynamic_cast<RMaterialBlinnPhong*>(LightInfo.Hit.Mat);
	if (!Mat)
	{
		Mat = GetDefaultMaterial();
	}

	const auto L = LightInfo.GetLightVector();

	const double Distance = LightInfo.GetLightDistance();
	const double Attenuation = Distance * Distance;

	/* Diffuse lighting */
	const double Lambertian = std::max(L | LightInfo.Hit.Normal, 0.0);

	/* Specular highlights */
	double Specular = 0.0;
	if (Lambertian > 0.0)
	{
		const auto H = (L + LightInfo.View).Normalized();
		const double SpecularAngle = H | LightInfo.Hit.Normal;
		Specular = std::pow(SpecularAngle, Mat->SpecularExponent);
	}

	Vector3 Color = Mat->Color * Lambertian * LightInfo.GetLightColor() / Attenuation;
	Color += Specular * LightInfo.GetLightColor() / Attenuation;

	
	return BRDFResult(Color / PI, 1.0);
}
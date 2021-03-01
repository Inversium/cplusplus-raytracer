#pragma once

#include "ShadingModel.h"
#include "Scene.h"
#include "Material.h"
#include "Core.h"


class BlinnPhongModel : public ShadingModel
{
public:
	virtual Vector3 Light(const RLightInfo& LightInfo, std::function<bool(const RRay&, RHit&)> QueryScene) const override;

private:
	virtual class RMaterialBlinnPhong* GetDefaultMaterial() const override;

};

inline RMaterialBlinnPhong* BlinnPhongModel::GetDefaultMaterial() const
{
	return new RMaterialBlinnPhong(Vector3(0.0), 1.0);
}


inline Vector3 BlinnPhongModel::Light(const RLightInfo& LightInfo, std::function<bool(const RRay&, RHit&)> QueryScene) const
{
	auto Mat = dynamic_cast<RMaterialBlinnPhong*>(LightInfo.Hit.Mat);
	if (!Mat)
	{
		Mat = GetDefaultMaterial();
	}

	const auto L = LightInfo.GetLightVector();

	const double Distance = (LightInfo.Hit.Position - LightInfo.GetLightPosition()).Length();
	const double Attenuation = Distance * Distance;

	const double Lambertian = std::max(L | LightInfo.Hit.Normal, 0.0);

	double Specular = 0.0;
	if (Lambertian > 0.0)
	{
		const auto H = (L + LightInfo.View).Normalized();
		const double SpecularAngle = H | LightInfo.Hit.Normal;
		Specular = std::pow(SpecularAngle, Mat->SpecularExponent);
	}

	Vector3 Color = Mat->Color * Lambertian * LightInfo.GetLightColor() / Attenuation;
	Color = Color + Specular * LightInfo.GetLightColor() / Attenuation;

	
	return Color;
}
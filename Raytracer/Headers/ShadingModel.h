#pragma once

#include "math/Vector.h"

class RScene;

struct BRDFResult
{
	BRDFResult(const Vector3 InColor, const Vector3 InF0, const Vector3& InDiffuse, const Vector3& InSpecular) : Color(InColor), F0(InF0), Diffuse(InDiffuse), Specular(InSpecular)  {}

	Vector3 Color;
	Vector3 F0;
	Vector3 Diffuse;
	Vector3 Specular;
};

struct RLightInfo
{
	Vector3 Normal;
	Vector3 View;
	Vector3 Light;
	const SharedPtr<const class RMaterial> Mat;

public:
	RLightInfo(const Vector3& InNormal, const Vector3& InView, const Vector3& InLight, const SharedPtr<const RMaterial> InMat) : Normal(InNormal), View(InView), Light(InLight), Mat(InMat) {}
};

class BRDF
{

public:
	virtual BRDFResult Get(const RLightInfo& LightInfo) const = 0;

	virtual double GetPDF(const RLightInfo& LightInfo) const { return 1.0 / (2.0 * PI); };
	virtual Vector3 Sample(const RLightInfo& LightInfo) const 
	{
		return GetRandomVectorHemisphere(LightInfo.Normal);
	}

private:
	virtual RMaterial GetDefaultMaterial() const = 0;
};


#pragma once

#include "math/Vector.h"


struct RMaterial
{
protected:
	RMaterial() {};

	virtual ~RMaterial() {};
};

struct RMaterialBlinnPhong : RMaterial
{
	Vector3 Color;
	double SpecularExponent;

	RMaterialBlinnPhong(const Vector3& InColor, const double InSpecularExponent)
		: Color(InColor), SpecularExponent(InSpecularExponent) {}
};


struct RMaterialPBR : public RMaterial
{
	Vector3 Color;
	double Roughness = 0.5;
	double Metallic = 0.0;
	double RefractiveIndex = 1.0;
	double Transmission = 0.0;
	Vector3 Emissive;

	RMaterialPBR() = default;

	RMaterialPBR(const Vector3 InColor, const double InRoughness, const double InMetallic, const double InRefractiveIndex, const double InTransmission, const Vector3 InEmissive) :
		Color(InColor),
		Roughness(InRoughness),
		Metallic(InMetallic),
		RefractiveIndex(InRefractiveIndex),
		Transmission(InTransmission),
		Emissive(InEmissive) {}

	static const RMaterialPBR Metal;
	static const RMaterialPBR RedPlastic;
	static const RMaterialPBR BluePlastic;
	static const RMaterialPBR YellowRubber;
	static const RMaterialPBR Mirror;
	static const RMaterialPBR Glass;
	static const RMaterialPBR Diamond;
};
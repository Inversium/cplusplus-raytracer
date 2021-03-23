#pragma once

#include "math/Vector.h"


struct RMaterial
{
	Vector3 Color;

protected:
	RMaterial() {};
	RMaterial(const Vector3& InColor) : Color(InColor) {};

	virtual ~RMaterial() {};
};

struct RMaterialBlinnPhong : public RMaterial
{
	
	double SpecularExponent;

	RMaterialBlinnPhong(const Vector3& InColor, const double InSpecularExponent)
		: RMaterial(InColor), SpecularExponent(InSpecularExponent) {}
};


struct RMaterialPBR : public RMaterial
{
	double Roughness = 0.5;
	double Metallic = 0.0;
	double RefractiveIndex = 1.0;
	double Transmission = 0.0;
	Vector3 Emissive;

	RMaterialPBR() = default;

	RMaterialPBR(const Vector3 InColor, const double InRoughness, const double InMetallic, const double InRefractiveIndex, const double InTransmission, const Vector3 InEmissive) :
		RMaterial(InColor),
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
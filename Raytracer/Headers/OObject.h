#pragma once
#include "math/mvector.h"

struct RRay
{
	Vector3 Origin = Vector3(0.0, 0.0, 0.0);
	Vector3 Direction = Vector3(0.0, 0.0, -1.0);

	RRay() = default;
};

struct RMaterial
{
	Vector3 Color;
	double Roughness = 0.5;
	double Metallic = 0.0;
	double RefractiveIndex = 1.0;
	double Transmission = 0.0;
	Vector3 Emissive;

	RMaterial() = default;

	RMaterial(const Vector3 InColor, const double InRoughness, const double InMetallic, const double InRefractiveIndex, const double InTransmission, const Vector3 InEmissive) :
		Color(InColor),
		Roughness(InRoughness),
		Metallic(InMetallic),
		RefractiveIndex(InRefractiveIndex),
		Transmission(InTransmission),
		Emissive(InEmissive) {}

	static const RMaterial Metal;
	static const RMaterial RedPlastic;
	static const RMaterial BluePlastic;
	static const RMaterial YellowRubber;
	static const RMaterial Mirror;
	static const RMaterial Glass;
	static const RMaterial Diamond;
};


struct RHit
{
	Vector3 Position;
	Vector3 Normal;
	RMaterial Mat;
	double Depth = (double)LLONG_MAX;
};

class OObject
{
public:
	RMaterial Mat;
	Vector3 Position;

	virtual bool Intersects(const RRay Ray, RHit& OutHit) const { return false; }
};

class OSphere : public OObject
{
public:
	double Radius;

	OSphere() : Radius(5.0) {}

	virtual bool Intersects(const RRay Ray, RHit& OutHit) const override;
};

class OPlane : public OObject
{
public:
	Vector3 Normal;

	OPlane() : Normal(Vector3(0.0, 1.0, 0.0)) {}

	virtual bool Intersects(const RRay Ray, RHit& OutHit) const override;
};

/* Axis Aligned Bounding Box class, it uses the position member as origin and Extent as the actual extent */
class OBox : public OObject
{
public:
	Vector3 Extent;

	virtual bool Intersects(const RRay Ray, RHit& OutHit) const override;
	void SetByMinMax(const Vector3& VMin, const Vector3& VMax);
};

class OLight : public OSphere
{
public:
	Vector3 Color = Vector3(1.0, 1.0, 1.0);

	OLight() = default;
};
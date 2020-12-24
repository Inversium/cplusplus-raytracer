#pragma once
#include "math/Headers/mvector.h"

struct RLight
{
	Vector3 Position = Vector3(0.0, 0.0, 0.0);
	Vector3 Color = Vector3(1.0, 1.0, 1.0);

	RLight() = default;
};

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

	RMaterial() = default;

	RMaterial(const Vector3 InColor, const double InRoughness, const double InMetallic) :
		Color(InColor),
		Roughness(InRoughness),
		Metallic(InMetallic) {}

	static const RMaterial Metal;
	static const RMaterial RedPlastic;
	static const RMaterial BluePlastic;
	static const RMaterial YellowRubber;
};
const RMaterial RMaterial::Metal = RMaterial(Vector3(1.0), 0.25, 1.0);
const RMaterial RMaterial::RedPlastic = RMaterial(Vector3(1.0, 0.0, 0.0), 0.0, 0.0);
const RMaterial RMaterial::YellowRubber = RMaterial(Vector3(1.0, 1.0, 0.0), 1.0, 0.0);
const RMaterial RMaterial::BluePlastic = RMaterial(Vector3(0.1, 0.1, 1.0), 0.0, 0.0);

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

	OSphere() : Radius(5.) {}

	virtual bool Intersects(const RRay Ray, RHit& OutHit) const override
	{
		const Vector3 L = (Position - Ray.Origin); //Vector from Ray origin to Sphere position

		const double tca = L | Ray.Direction.Normalized();
		if (tca < 0) return false;

		const double d2 = (L | L) - tca * tca; //Distance from Sphere position to ray
		if (d2 > Radius * Radius) return false;

		const double HalfInner = sqrt(Radius * Radius - d2); //half of the ray length inside sphere

		double t0 = tca - HalfInner;
		double t1 = tca + HalfInner;	

		if (t0 < 0)
		{
			t0 = t1;
			if (t0 < 0) return false;
		}

		const Vector3 HitPoint = Ray.Origin + Ray.Direction * t0;
		OutHit.Mat = this->Mat;
		OutHit.Normal = (HitPoint - Position).Normalized();
		OutHit.Position = HitPoint;
		OutHit.Depth = t0;

		return true;
	}
};

class OPlane : public OObject
{
public:
	Vector3 Normal;

	OPlane() : Normal(Vector3(0.0, 1.0, 0.0)) {}

	virtual bool Intersects(const RRay Ray, RHit& OutHit) const override
	{
		const double Denom = Normal | Ray.Direction;
		if (abs(Denom) > 1e-10)
		{
			const double T = ((Position - Ray.Origin) | Normal) / Denom;
			if (T >= 1e-5)
			{
				OutHit.Mat = this->Mat;
				OutHit.Normal = this->Normal;
				OutHit.Position = Ray.Origin + Ray.Direction * T;
				OutHit.Depth = T;
				return true;
			}
		}
		return false;
	}
};
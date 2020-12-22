#pragma once
#include "math/Headers/mvector.h"

struct RLight
{
	vec3 Position = vec3(0.0, 0.0, 0.0);
	vec3 Color = vec3(1.0, 1.0, 1.0);

	RLight() = default;
};

struct RRay
{
	vec3 Origin = vec3(0.0, 0.0, 0.0);
	vec3 Direction = vec3(0.0, 0.0, -1.0);

	RRay() = default;
};

struct RMaterial
{
	vec3 Color;
	double Roughness = 0.5;
	double Metallic = 0.0;

	RMaterial() = default;

	RMaterial(const vec3 InColor, const double InRoughness, const double InMetallic) :
		Color(InColor),
		Roughness(InRoughness),
		Metallic(InMetallic) {}

	static const RMaterial Metal;
	static const RMaterial RedPlastic;
	static const RMaterial BluePlastic;
	static const RMaterial YellowRubber;
};
const RMaterial RMaterial::Metal = RMaterial(vec3(1.0), 0.25, 1.0);
const RMaterial RMaterial::RedPlastic = RMaterial(vec3(1.0, 0.0, 0.0), 0.0, 0.0);
const RMaterial RMaterial::YellowRubber = RMaterial(vec3(1.0, 1.0, 0.0), 1.0, 0.0);
const RMaterial RMaterial::BluePlastic = RMaterial(vec3(0.1, 0.1, 1.0), 0.0, 0.0);

struct RHit
{
	vec3 Position;
	vec3 Normal;
	RMaterial Mat;
	double Depth = (double)LLONG_MAX;
};

class OObject
{
public:
	RMaterial Mat;
	vec3 Position;

	virtual bool Intersects(const RRay Ray, RHit& OutHit) { return false; }
};

class OSphere : public OObject
{
public:
	double Radius;

	OSphere() : Radius(5.) {}

	virtual bool Intersects(const RRay Ray, RHit& OutHit) override
	{
		const vec3 L = (Position - Ray.Origin); //Vector from Ray origin to Sphere position

		const double tca = L | Ray.Direction.normalized();
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

		const vec3 HitPoint = Ray.Origin + Ray.Direction * t0;
		OutHit.Mat = this->Mat;
		OutHit.Normal = (HitPoint - Position).normalized();
		OutHit.Position = HitPoint;
		OutHit.Depth = t0;

		return true;
	}
};

class OPlane : public OObject
{
public:
	vec3 Normal;

	OPlane() : Normal(vec3(0.0, 1.0, 0.0)) {}

	virtual bool Intersects(const RRay Ray, RHit& OutHit) override
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
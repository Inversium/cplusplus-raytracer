#pragma once
#include "math/Headers/mvector.h"

struct Ray
{
	vec3 Origin;
	vec3 Direction;
	vec3 Intensity;

	Ray() : Origin(0., 0., 0.), Direction(0., 0., -1.), Intensity(1., 1., 1.) {}
};

struct Material
{
	vec3 Color;

	Material() : Color(0., 0., 0.) {}
};

struct Hit
{
	vec3 Position;
	vec3 Normal;
	Material Mat;
};

class Object
{
public:
	Material Mat;
	vec3 Position;

	virtual bool Intersects(Ray ray, Hit& OutHit) { return false; }
};

class Sphere : public Object
{
public:
	double Radius;

	Sphere() : Radius(0.) {}

	virtual bool Intersects(Ray ray, Hit& OutHit) override
	{
		vec3 pos = (Position - ray.Origin); //Vector from Ray origin to Sphere position

		double tca = pos | ray.Direction.normalized();
		if (tca < 0) return false;

		double d = sqrt(pow(pos.getLength(), 2) - pow(tca, 2)); //Distance from Sphere position to ray
		if (d > Radius) return false;

		double HalfInner = sqrt(Radius * Radius - d * d); //half of the ray length inside sphere

		vec3 P1 = ray.Origin + ray.Direction.normalized() * ((pos | ray.Direction.normalized()) - HalfInner);
		vec3 P2 = ray.Origin + ray.Direction.normalized() * ((pos | ray.Direction.normalized()) + HalfInner);

		

		if ((P1 | ray.Origin) < 0)
		{
			P1 = P2;
			if ((P1 | ray.Origin) < 0) return false;
		}

		OutHit.Mat = this->Mat;
		OutHit.Normal = (P1 - Position).normalized();
		OutHit.Position = P1;
		return true;
	}
};


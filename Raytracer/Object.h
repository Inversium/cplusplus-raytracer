#pragma once
#include "math/Headers/mvector.h"

struct Light
{
	vec3 Position = vec3(0., 0., 0.);
	vec3 Color = vec3(1., 1., 1.);
	double Intensity = 0;
};

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
	double Roughness = 0.5;
	double Specular = 0.5;
	double Albedo = 1.0;

	Material() : Color(0., 0., 0.) {}
};

struct Hit
{
	vec3 Position;
	vec3 Normal;
	Material Mat;
	double Depth;
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

	Sphere() : Radius(5.) {}

	virtual bool Intersects(Ray ray, Hit& OutHit) override
	{
		double t0, t1;
		vec3 L = (Position - ray.Origin); //Vector from Ray origin to Sphere position

		double tca = L | ray.Direction.normalized();
		if (tca < 0) return false;

		double d2 = (L | L) - tca * tca; //Distance from Sphere position to ray
		if (d2 > Radius * Radius) return false;

		double HalfInner = sqrt(Radius * Radius - d2); //half of the ray length inside sphere

		t0 = tca - HalfInner;
		t1 = tca + HalfInner;	

		if (t0 < 0)
		{
			t0 = t1;
			if (t0 < 0) return false;
		}

		vec3 HitPoint = ray.Origin + ray.Direction * t0;
		OutHit.Mat = this->Mat;
		OutHit.Normal = (HitPoint - Position).normalized();
		OutHit.Position = HitPoint;
		OutHit.Depth = t0;

		return true;
	}
};


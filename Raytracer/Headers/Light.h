#pragma once
#include <memory>
#include "OObject.h"
#include "Random.h"
#include "math/Math.h"


class RLight : public RPrimitive
{
protected:
	Vector3 Color = Vector3(1.0);
	RLight() {}

public:
	void SetColor(const Vector3& InColor);
	Vector3 GetColor() const { return Color; }
	virtual Vector3 SampleDirection(const Vector3& Point) const = 0;
	virtual double Area() const = 0;
};

class RSphereLight final : public RLight
{
	double Radius;

public:
	RSphereLight() : Radius(0.0) {}
	RSphereLight(const Vector3& InColor, const double InRadius)
		: Radius(InRadius)
	{
		SetColor(InColor);
	}

	virtual bool Intersects(const RRay& Ray, RHit& OutHit) const override;
	virtual AABB GetBoundingBox() const override;
	virtual Vector3 SampleDirection(const Vector3& Point) const override;
	virtual double Area() const override;
};

inline void RLight::SetColor(const Vector3& InColor)
{
	Color = InColor;
	Mat->InitializeLight(InColor);
}

inline bool RSphereLight::Intersects(const RRay& Ray, RHit& OutHit) const
{
	RRay LocalRay;
	LocalRay.Direction = Transform.InverseTransformVector(Ray.Direction).Normalized();
	LocalRay.Origin = Transform.InverseTransformPosition(Ray.Origin);
	const Vector3 L = -LocalRay.Origin; //Vector from Ray origin to Sphere position

	const double tca = L | LocalRay.Direction;
	if (tca < 0) return false;

	const double d2 = (L | L) - tca * tca; //Distance from Sphere position to ray
	if (d2 > Radius * Radius) return false;

	const double HalfInner = sqrt(Radius * Radius - d2); //half of the ray length inside sphere

	double t0 = tca - HalfInner;
	double t1 = tca + HalfInner;

	bool bInside = false;
	if (t0 < 0)
	{
		bInside = true;
		t0 = t1;
		if (t0 < 0) return false;
	}

	const Vector3 LocalHitPoint = LocalRay.Origin + LocalRay.Direction * t0;
	const Vector3 HitPoint = Transform.TransformPosition(LocalHitPoint);

	OutHit.Mat = GetMaterial();
	OutHit.Normal = bInside ? -(HitPoint - Transform.GetPosition()).Normalized() : (HitPoint - Transform.GetPosition()).Normalized();
	OutHit.Position = HitPoint;
	OutHit.Depth = (HitPoint - Ray.Origin).Length();
	OutHit.Object = shared_from_this();

	return true;
}

inline AABB RSphereLight::GetBoundingBox() const
{	
	return AABB(Transform.GetPosition() + Vector3(-Radius), Transform.GetPosition() + Vector3(Radius));
}

inline Vector3 RSphereLight::SampleDirection(const Vector3& Point) const
{
	const double ConeAngle = GetConeAngleByFittingSphere(Radius, (Point - Transform.GetPosition()).Length());
	const Vector3 ConeDir = (Transform.GetPosition() - Point).Normalized();
	return GetRandomUnitVectorInsideCone(ConeDir, ConeAngle);
}

inline double RSphereLight::Area() const
{
	return PI * Radius * Radius;
}

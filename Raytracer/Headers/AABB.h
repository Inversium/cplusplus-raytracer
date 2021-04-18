#pragma once

#include "Core.h"

/* Axis Aligned Bounding Box class */
struct AABB
{
	Vector3 Min;
	Vector3 Max;

	AABB() : Min(0.0), Max(0.0) {}
	AABB(const Vector3 InMin, const Vector3 InMax) : Min(InMin), Max(InMax) {}

	Vector3 GetExtent() const
	{
		return ((Max - Min) / 2.0).Abs();
	}

	Vector3 GetPosition() const
	{
		return (Max + Min) / 2.0;
	}

	void SetByCenterAndExtent(const Vector3& Center, const Vector3& Extent)
	{
		Min = Center - Extent;
		Max = Center + Extent;
	}

	double Area() const
	{
		const double SideX = Max.X - Min.X;
		const double SideY = Max.Y - Min.Y;
		const double SideZ = Max.Z - Min.Z;

		return 2.0 * (SideX * SideY + SideY * SideZ + SideZ * SideX);
	}

	bool Intersects(const RRay Ray) const
	{
		const Vector3 LocalRayOrigin = Ray.Origin - GetPosition();
		const Vector3 m = Vector3(1.0) / Ray.Direction;
		const Vector3 n = m * LocalRayOrigin;
		const Vector3 k = m.Abs() * GetExtent();
		const Vector3 t1 = -n - k;
		const Vector3 t2 = -n + k;

		const double tN = t1.GetMax(); //Near point distance
		const double tF = t2.GetMin(); //Far point distance

		if (tN > tF || tF < 0.0) return false;

		return true;
	}
};
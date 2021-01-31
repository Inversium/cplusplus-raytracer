#include "../Headers/OObject.h"

const RMaterial RMaterial::Metal = RMaterial(Vector3(1.0), 0.25, 1.0, 1.0, 0.0, Vector3(0.0));
const RMaterial RMaterial::RedPlastic = RMaterial(Vector3(1.0, 0.0, 0.0), 0.0, 0.0, 1.0, 0.0, Vector3(0.0));
const RMaterial RMaterial::YellowRubber = RMaterial(Vector3(1.0, 1.0, 0.0), 1.0, 0.0, 1.0, 0.0, Vector3(0.0));
const RMaterial RMaterial::BluePlastic = RMaterial(Vector3(0.1, 0.1, 1.0), 0.0, 0.0, 1.0, 0.0, Vector3(0.0));
const RMaterial RMaterial::Mirror = RMaterial(Vector3(1.0), 0.0, 1.0, 1.0, 0.0, Vector3(0.0));
const RMaterial RMaterial::Glass = RMaterial(Vector3(1.0), 0.0, 0.0, 1.458, 1.0, Vector3(0.0));
const RMaterial RMaterial::Diamond = RMaterial(Vector3(1.0), 0.0, 0.0, 2.417, 1.0, Vector3(0.0));


bool OBox::Intersects(const RRay Ray, RHit& OutHit) const
{
	const Vector3 LocalRayOrigin = Ray.Origin - Position;
	const Vector3 m = Vector3(1.0) / Ray.Direction;
	const Vector3 n = m * LocalRayOrigin;
	const Vector3 k = Vector3::Abs(m) * Extent;
	const Vector3 t1 = -n - k;
	const Vector3 t2 = -n + k;

	const double tN = t1.GetMax(); //Near point distance
	const double tF = t2.GetMin(); //Far point distance

	if (tN > tF || tF < 0.0) return false;


	//If the ray origin is inside the box (tN < 0) the actual intersection position will be at (RayDir * tF)
	//Same goes for Depth
	const bool bInsideBox = tN < 0.0;
	OutHit.Position = Position + (bInsideBox ? LocalRayOrigin + Ray.Direction * tF : LocalRayOrigin + Ray.Direction * tN);
	OutHit.Depth = bInsideBox ? tF : tN;

	OutHit.Mat = Mat;

	const Vector3 LocalHit = OutHit.Position - Position;
	OutHit.Normal = {
		std::trunc(LocalHit.X / Extent.X * (1.0 + 1e-8)),
		std::trunc(LocalHit.Y / Extent.Y * (1.0 + 1e-8)),
		std::trunc(LocalHit.Z / Extent.Z * (1.0 + 1e-8))
	};
	if (bInsideBox) OutHit.Normal = -OutHit.Normal;

	return true;
}

void OBox::SetByMinMax(const Vector3& VMin, const Vector3& VMax)
{
	Position = (VMax + VMin) / 2;
	Extent = (VMax - VMin) / 2;
}

bool OSphere::Intersects(const RRay Ray, RHit& OutHit) const
{
	const Vector3 L = (Position - Ray.Origin); //Vector from Ray origin to Sphere position

	const double tca = L | Ray.Direction;
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

	const Vector3 HitPoint = Ray.Origin + Ray.Direction * t0;
	OutHit.Mat = this->Mat;
	OutHit.Normal = bInside ? -(HitPoint - Position).Normalized() : (HitPoint - Position).Normalized();
	OutHit.Position = HitPoint;
	OutHit.Depth = t0;

	return true;
}

bool OPlane::Intersects(const RRay Ray, RHit& OutHit) const
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
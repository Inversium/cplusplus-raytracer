#pragma once

#include <sstream>
#include <vector>
#include <fstream>

#include "math/Vector.h"
#include "CoreUtilities.h"
#include "Transform.h"
#include "Material.h"



struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;

	Vertex() : Position(0.0), Normal(0.0), UV(0.0) {}
	Vertex(const Vector3& InPosition, const Vector3& InNormal) : Position(InPosition), Normal(InNormal) {}
};

struct Triangle
{
	const Vertex* Vertices[3];

	Triangle()
	{
		Vertices[0] = nullptr;
		Vertices[1] = nullptr;
		Vertices[2] = nullptr;
	}
	Triangle(Vertex& V1, Vertex& V2, Vertex& V3)
	{
		Vertices[0] = &V1;
		Vertices[1] = &V2;
		Vertices[2] = &V3;
	}

	double Area() const 
	{ 
		const Vector3 Edge1 = Vertices[1]->Position - Vertices[0]->Position;
		const Vector3 Edge2 = Vertices[2]->Position - Vertices[0]->Position;
		return (Edge1 ^ Edge2).Length() / 2.0; 
	}
};

class OObject
{
public:
	RMaterial* Mat = nullptr;
	RTransform Transform;


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

/* Axis Aligned Bounding Box class */
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


class OMesh : public OObject
{
public:
	OMesh(std::string Path);
	OMesh() = default;

private:
	std::vector<Vertex> Vertices;
	std::vector<Triangle> Triangles;
	OBox AABB;

	Vertex GetVertex(uint32_t Index) const;
	Vertex GetVertex(uint32_t TriangleIndex, uint8_t LocalIndex) const;
	void UpdateAABB();

	bool TriangleIntersect(const uint32_t FaceIndex, const RRay Ray, RHit& OutHit) const;

public:
	bool LoadModel(std::string Path);
	uint32_t NVerts() const;
	uint32_t NFaces() const;

	virtual bool Intersects(const RRay Ray, RHit& OutHit) const;
};



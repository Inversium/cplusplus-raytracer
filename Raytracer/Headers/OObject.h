#pragma once

#include <sstream>
#include <vector>
#include <fstream>

#include "math/Vector.h"
#include "CoreUtilities.h"
#include "Transform.h"
#include "Material.h"
#include "AABB.h"



struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;

	Vertex() : Position(0.0), Normal(0.0), UV(0.0) {}
	Vertex(const Vector3& InPosition, const Vector3& InNormal) : Position(InPosition), Normal(InNormal) {}
};


class RPrimitive : public std::enable_shared_from_this<RPrimitive>
{
protected:
	SharedPtr<RMaterial> Mat = nullptr;

public:	
	RTransform Transform;

	RPrimitive()
	{
		Mat = MakeShared<RMaterial>();
	}

	virtual AABB GetBoundingBox() const = 0;
	virtual bool Intersects(const RRay& Ray, RHit& OutHit) const = 0;
	virtual SharedPtr<RMaterial> GetMaterial() const { return Mat; };
	virtual void SetMaterial(SharedPtr<RMaterial> NewMaterial) { Mat = NewMaterial; };
};

class Triangle : public RPrimitive
{
	SharedPtr<Vertex> Vertices[3];
	bool bSmoothShading;

public:
	Triangle()
	{
		Vertices[0] = nullptr;
		Vertices[1] = nullptr;
		Vertices[2] = nullptr;
		bSmoothShading = true;
	}
	Triangle(SharedPtr<Vertex>& V1, SharedPtr<Vertex>& V2, SharedPtr<Vertex>& V3, const bool InbSmoothShading = true)
	{
		Vertices[0] = V1;
		Vertices[1] = V2;
		Vertices[2] = V3;
		bSmoothShading = InbSmoothShading;
	}

	const SharedPtr<Vertex> GetVertex(const uint8_t Index) const
	{
		return Vertices[Index];
	}

	void SetVertex(const uint8_t Index, SharedPtr<Vertex>& InVertex)
	{
		Vertices[Index] = InVertex;
	}

	double Area() const
	{
		const Vector3 Edge1 = Vertices[1]->Position - Vertices[0]->Position;
		const Vector3 Edge2 = Vertices[2]->Position - Vertices[0]->Position;
		return (Edge1 ^ Edge2).Length() / 2.0;
	}

	//Normalized normal of the triangle
	Vector3 Normal() const
	{
		const Vector3 Edge1 = Vertices[1]->Position - Vertices[0]->Position;
		const Vector3 Edge2 = Vertices[2]->Position - Vertices[0]->Position;
		return (Edge1 ^ Edge2).Normalized();
	}

	//Unnormalized normal of the triangle, just the cross product
	Vector3 RawNormal() const
	{
		const Vector3 Edge1 = Vertices[1]->Position - Vertices[0]->Position;
		const Vector3 Edge2 = Vertices[2]->Position - Vertices[0]->Position;
		return (Edge1 ^ Edge2);
	}

	
	virtual AABB GetBoundingBox() const override;
	virtual bool Intersects(const RRay& Ray, RHit& OutHit) const override;
};

class OSphere : public RPrimitive
{
public:
	double Radius;

	OSphere() : Radius(5.0) {}

	virtual AABB GetBoundingBox() const override
	{ 
		return AABB(Transform.GetPosition() + Vector3(-Radius), Transform.GetPosition() + Vector3(Radius)); 
	}
	virtual bool Intersects(const RRay& Ray, RHit& OutHit) const override;
};

class OPlane : public RPrimitive
{
public:
	Vector3 Normal;

	OPlane() : Normal(Vector3(0.0, -1.0, 0.0)) {}

	virtual AABB GetBoundingBox() const override
	{ 
		Vector3 Min(-1000.0, -1000.0, -0.01);
		Vector3 Max( 1000.0,  1000.0,  0.01);
		Min = TransformToWorld(Min, Normal);
		Max = TransformToWorld(Max, Normal);
		return AABB(Min, Max); 
	}
	virtual bool Intersects(const RRay& Ray, RHit& OutHit) const override;
};


class OBox : public RPrimitive
{
public:
	Vector3 Extent;

	OBox() {}

	virtual bool Intersects(const RRay& Ray, RHit& OutHit) const override;
	void SetByMinMax(const Vector3& VMin, const Vector3& VMax);
};





class OMesh : public RPrimitive
{
public:
	OMesh(const char* Path);
	OMesh() {};

private:
	std::vector<SharedPtr<Vertex>> Vertices;
	std::vector<SharedPtr<Triangle>> Triangles;
	AABB BBox;

	/* Call when the model's vertices/triangles was modified */
	void UpdateAABB();
	void UpdateSmoothNormals();

public:
	bool LoadModel(const std::string& Path);
	size_t CountVerts() const { return Vertices.size(); }
	size_t CountFaces() const { return Triangles.size(); }

	virtual AABB GetBoundingBox() const { return BBox; }
	virtual bool Intersects(const RRay& Ray, RHit& OutHit) const;

	friend class RScene;
};



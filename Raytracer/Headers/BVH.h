#pragma once

#include <vector>
#include <stack>

#include "Core.h"
#include "AABB.h"
#include "OObject.h"
#include "Scene.h"

struct BVHNodeBase
{
	AABB Box;
	virtual bool IsLeaf() const = 0;
};

struct BVHNode final : public BVHNodeBase
{
	UniquePtr<BVHNodeBase> Left;
	UniquePtr<BVHNodeBase> Right;

	virtual bool IsLeaf() const { return false; }
};

struct BVHLeaf final : public BVHNodeBase
{
	std::vector<SharedPtr<RPrimitive>> Objects;

	virtual bool IsLeaf() const { return true; }
};

/* Helper struct for BVH constructing which contains primitive and its cached AABB */
struct BoxPrimitive
{
	AABB Box;
	const SharedPtr<RPrimitive> Primitive;

	BoxPrimitive(const AABB& InBox, const SharedPtr<RPrimitive> InPrimitive) : Box(InBox), Primitive(InPrimitive) {}
};


/* Compare components in two vectors and assign values based on Compare Function to AssignTo vector */
template<typename CompareFunc>
inline void AssignVector(Vector3& AssignTo, const Vector3& CompareTo, CompareFunc Func)
{
	AssignTo.X = Func(CompareTo.X, AssignTo.X) ? CompareTo.X : AssignTo.X;
	AssignTo.Y = Func(CompareTo.Y, AssignTo.Y) ? CompareTo.Y : AssignTo.Y;
	AssignTo.Z = Func(CompareTo.Z, AssignTo.Z) ? CompareTo.Z : AssignTo.Z;
}
 
/* Get minimal AABB to include all AABB in BoxList */
inline void GetObjectsAABB(Vector3& Min, Vector3& Max, const std::vector<BoxPrimitive>& BoxList)
{
	Max = { -DBL_MAX, -DBL_MAX, -DBL_MAX };
	Min = {  DBL_MAX,  DBL_MAX,  DBL_MAX };

	for (auto& Box : BoxList) 
	{		
		AssignVector(Min, Box.Box.Min, std::less<double>());
		AssignVector(Max, Box.Box.Max, std::greater<double>());
	}
}


inline UniquePtr<BVHNodeBase> Recurse(std::vector<BoxPrimitive>& BoxList, uint16_t Depth = 0)
{
	if (BoxList.size() < 4) {
		auto Leaf = MakeUnique<BVHLeaf>();
		for (auto& Box : BoxList)
			Leaf->Objects.push_back(Box.Primitive);
		return Leaf;
	}

	// Start by finding the working list's bounding box
	Vector3 Min, Max;
	GetObjectsAABB(Min, Max, BoxList);

	// The current box has a cost of (No of triangles)*surfaceArea
	double MinCost = BoxList.size() * AABB(Min, Max).Area();
	double BestSplit = DBL_MAX; // will indicate no split with better cost found (below)
	int32_t BestAxis = -1;

	// Try all different axis
	for (uint8_t Axis = 0; Axis < 3; Axis++) 
	{
		// we will try dividing the triangles based on the current axis,
		// and we will try split values from "start" to "stop", one "step" at a time.
		double Start, Stop, Step;

		if (Axis == 0) 
		{
			Start = Min.X;
			Stop = Max.X;
		}
		else if (Axis == 1) 
		{
			Start = Min.Y;
			Stop = Max.Y;
		}
		else 
		{
			Start = Min.Z;
			Stop = Max.Z;
		}

		// In that axis, do the bounding boxes in the work queue "span" across?
		// Or are they all already "packed" on the axis's plane?
		if (std::abs(Stop - Start) < 1e-4)
			// No, we must move to a different axis!
			continue;

		// Try splitting at a uniform sampling that gets smaller the deeper we go:
		// size of "sampling grid": 1024 (depth 0), 512 (depth 1), etc
		Step = (Stop - Start) / (1024.0 / (Depth + 1.0));

		for (double TestSplit = Start + Step; TestSplit < Stop - Step; TestSplit += Step)
		{
			// The left and right bounding box
			Vector3 LeftMin(DBL_MAX, DBL_MAX, FLT_MAX), LeftMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
			Vector3 RightMin(DBL_MAX, DBL_MAX, FLT_MAX), RightMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);

			// The number of primitives in the left and right bboxes
			uint32_t CountLeft = 0, CountRight = 0;

			// For each test split, allocate primitives based on their bounding boxes centers
			for (auto& Box : BoxList) 
			{
				double Value;
				if (Axis == 0) Value = Box.Box.GetPosition().X;
				else if (Axis == 1) Value = Box.Box.GetPosition().Y;
				else Value = Box.Box.GetPosition().Z;

				if (Value < TestSplit) 
				{
					AssignVector(LeftMin, Box.Box.Min, std::less<double>());
					AssignVector(LeftMax, Box.Box.Max, std::greater<double>());
					CountLeft++;
				}
				else 
				{
					AssignVector(RightMin, Box.Box.Min, std::less<double>());
					AssignVector(RightMax, Box.Box.Max, std::greater<double>());
					CountRight++;
				}
			}

			// Now use the Surface Area Heuristic to see if this split has a better "cost"
			//
			// First, check for stupid partitionings
			if (CountLeft <= 1 || CountRight <= 1) continue;

			// It's a real partitioning, calculate the surface areas
			double TotalCost = AABB(LeftMin, LeftMax).Area() * CountLeft + AABB(RightMin, RightMax).Area() * CountRight;

			if (TotalCost < MinCost) 
			{
				MinCost = TotalCost;
				BestSplit = TestSplit;
				BestAxis = Axis;
			}
		}
	}

	// We found no split to improve the cost, create a BVH leaf
	if (BestAxis == -1) {
		auto Leaf = MakeUnique<BVHLeaf>();
		for (auto& Box : BoxList)
			Leaf->Objects.push_back(Box.Primitive);
		return Leaf;
	}

	// Create a BVH inner node, split with the optimal value we found above
	std::vector<BoxPrimitive> LeftBoxes, RightBoxes;
	Vector3 LeftMin(DBL_MAX, DBL_MAX, DBL_MAX), LeftMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	Vector3 RightMin(DBL_MAX, DBL_MAX, DBL_MAX), RightMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);

	for (auto& Box: BoxList) 
	{		
		double Value;
		if (BestAxis == 0) Value = Box.Box.GetPosition().X;
		else if (BestAxis == 1) Value = Box.Box.GetPosition().Y;
		else Value = Box.Box.GetPosition().Z;

		if (Value < BestSplit) 
		{
			LeftBoxes.push_back(Box);
			AssignVector(LeftMin, Box.Box.Min, std::less<double>());
			AssignVector(LeftMax, Box.Box.Max, std::greater<double>());
		}
		else 
		{
			RightBoxes.push_back(Box);
			AssignVector(RightMin, Box.Box.Min, std::less<double>());
			AssignVector(RightMax, Box.Box.Max, std::greater<double>());
		}
	}

	auto Inner = MakeUnique<BVHNode>();
	Inner->Left = Recurse(LeftBoxes, Depth + 1);
	Inner->Left->Box = AABB(LeftMin, LeftMax);
	Inner->Right = Recurse(RightBoxes, Depth + 1);
	Inner->Right->Box = AABB(RightMin, RightMax);

	return Inner;
}

inline UniquePtr<BVHNodeBase> CreateBVH(const RScene* Scene)
{
	std::vector<BoxPrimitive> BoxPrimitiveList;

	Vector3 Max(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	Vector3 Min( DBL_MAX,  DBL_MAX,  DBL_MAX);

	for (auto Primitive : Scene->GetPrimitives()) 
	{
		AABB Box = Primitive->GetBoundingBox();

		AssignVector(Min, Box.Min, std::less<double>());
		AssignVector(Max, Box.Max, std::greater<double>());

		BoxPrimitive B(Box, Primitive);
		BoxPrimitiveList.push_back(B);
	}

	auto Root = Recurse(BoxPrimitiveList);
	Root->Box.Max = Max;
	Root->Box.Min = Min;

	return Root;
}

inline bool BVHTraverse(const UniquePtr<BVHNodeBase>& Node, const RRay& Ray, RHit& OutHit)
{
	std::stack<const UniquePtr<BVHNodeBase>*> Stack;

	Stack.push(&Node);

	double MinDist = INFINITY;
	bool bHit = false;

	while (!Stack.empty())
	{
		auto Current = std::move(Stack.top());
		Stack.pop();

		if (!Current->get()->IsLeaf())
		{
			const auto NodeInner = dynamic_cast<const BVHNode*>(Current->get());
			if (NodeInner->Box.Intersects(Ray))
			{
				Stack.push(&NodeInner->Left);
				Stack.push(&NodeInner->Right);
			}
		}
		else
		{
			const auto Leaf = dynamic_cast<const BVHLeaf*>(Current->get());

			for (auto& Primitive : Leaf->Objects)
			{			
				RHit TempHit;
				if (Primitive->Intersects(Ray, TempHit) && TempHit.Depth < MinDist)
				{
					OutHit = TempHit;
					bHit = true;
					MinDist = TempHit.Depth;					
				}
			}
		}
	}

	return bHit;
}


inline uint32_t CountPrimitives(const UniquePtr<BVHNodeBase>& Node)
{
	if (Node->IsLeaf()) return dynamic_cast<const BVHLeaf*>(Node.get())->Objects.size();
	else
	{
		auto NodeInner = dynamic_cast<const BVHNode*>(Node.get());
		return CountPrimitives(NodeInner->Left) + CountPrimitives(NodeInner->Right);
	}
}

inline uint32_t CountLeaves(const UniquePtr<BVHNodeBase>& Node)
{
	if (Node->IsLeaf()) return 1;
	else
	{
		auto NodeInner = dynamic_cast<const BVHNode*>(Node.get());
		return CountLeaves(NodeInner->Left) + CountLeaves(NodeInner->Right);
	}
}
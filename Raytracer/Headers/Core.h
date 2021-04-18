#pragma once

#include "math/Vector.h"
#include "CoreUtilities.h"
#include <memory>

template<class T>
using UniquePtr = std::unique_ptr<T>;

template<class T>
using SharedPtr = std::shared_ptr<T>;


struct RRay
{
    RRay() {}
    RRay(const Vector3 InOrigin, const Vector3 InDirection) : Origin(InOrigin), Direction(InDirection) {}

    Vector3 Origin = Vector3(0.0, 0.0, 0.0);
    Vector3 Direction = Vector3(0.0, 0.0, -1.0);
};


struct RHit
{
    Vector3 Position;
    Vector3 Normal;
    SharedPtr<const class RMaterial> Mat = nullptr;
    double Depth = INFINITY;
    SharedPtr<const class RPrimitive> Object = nullptr;
};


template<class T>
inline SharedPtr<T> MakeShared()
{
    return std::make_shared<T>();
}

template<class T, class... Args>
inline SharedPtr<T> MakeShared(Args&&... args)
{
    return std::make_shared<T>(args...);
}

template<class T>
inline SharedPtr<T> MakeShared(size_t N)
{
    return std::make_shared<T>(N);
}


template<class T>
inline UniquePtr<T> MakeUnique()
{
    return std::make_unique<T>();
}

template<class T, class... Args>
inline UniquePtr<T> MakeUnique(Args&&... args)
{
    return std::make_unique<T>(args...);
}

template<class T>
inline UniquePtr<T> MakeUnique(size_t N)
{
    return std::make_unique<T>(N);
}


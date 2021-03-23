#pragma once

#include "math/Vector.h"
#include "CoreUtilities.h"
#include "Texture.h"


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
    class RMaterial* Mat = nullptr;
    double Depth = INFINITY;
};
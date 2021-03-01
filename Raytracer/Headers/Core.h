#pragma once

#include "math/Vector.h"
#include "CoreUtilities.h"
#include "Texture.h"


struct RRay
{
    Vector3 Origin = Vector3(0.0, 0.0, 0.0);
    Vector3 Direction = Vector3(0.0, 0.0, -1.0);

    RRay() = default;
};


struct RHit
{
    Vector3 Position;
    Vector3 Normal;
    class RMaterial* Mat = nullptr;
    double Depth = INFINITY;
};
#pragma once

#include <cstdint>
#include "math/Vector.h"

struct RHit;
struct RRay;
class RScene;

class RShader final
{
public:
	RShader();

private:
	bool bShadows;
	bool bGlobalIllumination;
	bool bTranslucency;
	uint16_t SamplesGI;
	uint8_t RayDepth;
	Vector3 BackgroundColor;

public:
	Vector3 Light(const RScene* const Scene, const RRay& Ray) const;

private:
	Vector3 LightInternal(const RScene* const Scene, const RRay& Ray, const uint8_t Depth) const;
};



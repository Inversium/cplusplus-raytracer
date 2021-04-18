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
	bool bIndirectSampling;
	bool bDirectSampling;
	bool bTranslucency;
	uint16_t SamplesIndirect;
	uint16_t SamplesDirect;
	uint8_t RayDepth;
	Vector3 BackgroundColor;

public:
	Vector3 Light(const RScene* const Scene, const RRay& Ray) const;

private:
	Vector3 LightInternal(const RScene* const Scene, const RRay& Ray) const;
	Vector3 RayRecurse(const RScene* const Scene, const RRay& Ray, const uint8_t Depth) const;

	Vector3 DirectLighting(const RScene* const Scene, const RRay& Ray, const RHit& Hit) const;
};



#pragma once
#include <cassert>
#include <cstdint>
#include <vector>


#include "Core.h"
#include "Texture.h"
#include "Color.h"


#define USE_BVH 1

class RLight;
class RPrimitive;
class BRDF;
class RShader;


class RScene
{
public:	
	RScene(const uint16_t InHeight, const uint16_t InWidth);
	~RScene();


	bool bSSAA;
	uint8_t SamplesSSAA;
	double FOV;

private:	
	
	/* HDR output of the scene render */
	UniquePtr<RTexture> RenderTexture = nullptr;

	/* Container with all scene objects */
	std::vector<SharedPtr<RPrimitive>> SceneObjects;

	/* Container with all scene lights */
	std::vector<SharedPtr<RLight>> SceneLights;

	UniquePtr<RTexture> EnvironmentTexture = nullptr;

	UniquePtr<RShader> Shader = nullptr;

	UniquePtr<BRDF> ModelBRDF = nullptr;

	mutable uint64_t TotalRaysShooted = 0;

#if USE_BVH
	UniquePtr<class BVHNodeBase> BVHRoot = nullptr;
#endif // USE_BVH

	


public:
	const RTexture* GetRenderTexture() const { return RenderTexture.get(); }

	void AddObject(SharedPtr<RPrimitive> Object);

	const std::vector<SharedPtr<RPrimitive>>& GetPrimitives() const { return SceneObjects; }
	
	void Render();

	void SetEnvironmentTexture(UniquePtr<RTexture>& Texture);

	bool QueryScene(const RRay& Ray, RHit& OutHit) const;

	void SetShader(UniquePtr<RShader> InShader);

	void SetBRDF(UniquePtr<BRDF> InBRDF);

private:

	Vector3 SampleEnvMap(const Vector3& Direction) const;
	void ExtractLightSources();

#if USE_BVH
	void BuildBVH();
#endif // USE_BVH

	
	
	RColor RenderPixel(const RRay& Ray) const;

	friend class RShader;
};
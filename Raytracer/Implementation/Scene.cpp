#include "../Headers/Scene.h"
#include "../Headers/CoreUtilities.h"
#include "../Headers/OObject.h"
#include "../Headers/ShadingModel.h"
#include "../Headers/Shader.h"
#include "../Headers/Light.h"
#include "../Headers/BVH.h"
#include <chrono>






RScene::RScene(const uint16_t InHeight, const uint16_t InWidth)
{
	RenderTexture = MakeUnique<RTexture>(InHeight, InWidth);

	bSSAA = false;
	SamplesSSAA = 4;
	FOV = DegToRad(90.0);
}

RScene::~RScene() = default;


void RScene::AddObject(SharedPtr<RPrimitive> Object)
{
	/* If object is a mesh, we want to add each its triangle as an individual object,
	 * so before that we need to copy mesh transform and material to all triangles 
	 * Probably not the best solution, should be reworked
	 */
	auto Mesh = std::dynamic_pointer_cast<OMesh>(Object);
	if (Mesh)
	{
		std::transform(Mesh->Triangles.begin(), Mesh->Triangles.end(), Mesh->Triangles.begin(),
			[Mesh](SharedPtr<Triangle> Tri)
			{
				Tri->Transform = Mesh->Transform;
				Tri->SetMaterial(Mesh->Mat);
				return Tri;
			});
		SceneObjects.insert(SceneObjects.end(), Mesh->Triangles.begin(), Mesh->Triangles.end());
	}
	else
	{
		SceneObjects.push_back(Object);
	}
}


Vector3 RScene::SampleEnvMap(const Vector3& Direction) const
{
	if (!EnvironmentTexture) return { 0.0, 0.0, 0.0 };

	Vector2 Polar = CartesianToPolar(Vector2(Direction.X, Direction.Y));
	const double U = ((Polar.Y / PI) + 1.0) * 0.5;
	const double V = (Direction.Z + 1.0) * 0.5;
	return EnvironmentTexture->GetByUV({ U, 1.0 - V }, true).ToVector();
}

void RScene::ExtractLightSources()
{
	SceneLights.clear();
	for(auto Primitive : SceneObjects)
	{
		auto Light = std::dynamic_pointer_cast<RLight>(Primitive);
		if (Light)
		{
			SceneLights.push_back(Light);
		}
	}
}

#if USE_BVH
void RScene::BuildBVH()
{
	LOG("Scene", LogType::LOG, "Start BVH building...");

	const auto StartTime = std::chrono::high_resolution_clock::now();
	BVHRoot = CreateBVH(this);
	const auto EndTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> DeltaTime = EndTime - StartTime;
	const double Time = DeltaTime.count() / 1000.0;
	LOG("Scene", LogType::LOG, "BVH was built in {:.2f} seconds", Time);
	LOG("Scene", LogType::LOG, "BVH has {} Primitives in {} Leaves", 
		CountPrimitives(BVHRoot), 
		CountLeaves(BVHRoot));
}
#endif

bool RScene::QueryScene(const RRay& Ray, RHit& OutHit) const
{
	TotalRaysShooted++;
#if USE_BVH
	return BVHTraverse(BVHRoot, Ray, OutHit);
#else
	double MinDist = INFINITY;
	bool bHit = false;

	for (auto Object : SceneObjects)
	{
		RHit TempHit;
		if (Object->Intersects(Ray, TempHit) && TempHit.Depth < MinDist)
		{
			bHit = true;
			MinDist = TempHit.Depth;
			OutHit = TempHit;
}
	}
	return bHit;
#endif
}

void RScene::Render()
{
	ExtractLightSources();

#if USE_BVH
	BuildBVH();
#endif

	const auto StartTime = std::chrono::high_resolution_clock::now();
	
	const auto Height = RenderTexture->GetHeight();
	const auto Width = RenderTexture->GetWidth();
	
	const double AspectRatio = static_cast<double>(Width) / Height;

	double JitterMatrix[4 * 2] = {
		-1.0 / 4.0,  3.0 / 4.0,
		3.0 / 4.0,  1.0 / 3.0,
		-3.0 / 4.0, -1.0 / 4.0,
		1.0 / 4.0, -3.0 / 4.0
	};

	const Vector3 CameraPosition(0.0, 0.0, 0.0);
	//Vector3 CameraDirection(1.0, 0.0, 0.0);

	int32_t CurrentPixel = 0;
	
	#pragma omp parallel for
	for (int32_t i = 0; i < Height; i++)
	{
		for (int32_t j = 0; j < Width; j++)
		{
			CurrentPixel++;
			RColor Pixel;
			if (bSSAA)
			{				
				for (int32_t k = 0; k < SamplesSSAA; k++)
				{
					/* Generate random numbers to shift ray inside a pixel */
					const double EpsilonX = Random::RDouble();
					const double EpsilonY = Random::RDouble();

					/* Screen-Space coords of a pixel [-1.0; 1.0] */
					double SSX = 2.0 * (j + EpsilonX) / static_cast<double>(Width) - 1.0;
					double SSY = 2.0 * (i + EpsilonY) / static_cast<double>(Height) - 1.0;
					SSX *= AspectRatio;

					const double PixelCameraX = SSX * tan(FOV / 2.0);
					const double PixelCameraY = SSY * tan(FOV / 2.0);

					RRay Ray;
					Ray.Origin = CameraPosition;
					Ray.Direction = Vector3(1.0, PixelCameraX, -PixelCameraY).Normalized();

					Pixel += RenderPixel(Ray);
				}
				Pixel = Pixel / SamplesSSAA;
			}
			else
			{
				/* Screen-Space coords of a pixel [-1.0; 1.0] */
				double SSX = 2.0 * (j + 0.5) / (double)Width - 1;
				double SSY = 2.0 * (i + 0.5) / (double)Height - 1;
				SSX *= AspectRatio;

				const double PixelCameraX = SSX * tan(FOV / 2.0);
				const double PixelCameraY = SSY * tan(FOV / 2.0);

				RRay Ray;
				Ray.Origin = CameraPosition;
				Ray.Direction = Vector3(1.0, PixelCameraX, -PixelCameraY).Normalized();

				Pixel = RenderPixel(Ray);
			}

			RenderTexture->Write(Pixel, j, i);
			
			#pragma omp critical
			{
				DrawPercent("Scene", "Rendering", CurrentPixel, Height * Width, 5);
			}

		}
	}

	const auto EndTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> DeltaTime = EndTime - StartTime;
	const double Time = DeltaTime.count() / 1000.0;
	LOG("Scene", LogType::LOG, "Rendering Time: {:.2f} seconds, total rays shooted: {}", Time, TotalRaysShooted);
}

void RScene::SetEnvironmentTexture(SharedPtr<RTexture>& Texture)
{
	EnvironmentTexture = Texture;
}

RColor RScene::RenderPixel(const RRay& Ray) const
{	
	return Shader->Light(this, Ray);
}

void RScene::SetShader(UniquePtr<RShader> InShader)
{
	Shader = std::move(InShader);
}

void RScene::SetBRDF(UniquePtr<BRDF> InBRDF)
{
	ModelBRDF = std::move(InBRDF);
}


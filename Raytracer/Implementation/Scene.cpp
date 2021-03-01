#include "../Headers/Scene.h"
#include "../Headers/CoreUtilities.h"
#include "../Headers/OObject.h"
#include "../Headers/Texture.h"
#include "../Headers/ShadingModel.h"
#include <chrono>






RScene::RScene(const uint16_t InHeight, const uint16_t InWidth) : Height(InHeight), Width(InWidth)
{
	SceneTexture = new RTexture<Vector3>(Height, Width);		
	NormalTexture = new RTexture<Vector3>(Height, Width);
	BaseColorTexture = new RTexture<Vector3>(Height, Width);
	EmissiveTexture = new RTexture<Vector3>(Height, Width);

	RoughnessTexture = new RTexture<double>(Height, Width);
	MetallicTexture = new RTexture<double>(Height, Width);
	TransmissionTexture = new RTexture<double>(Height, Width);
	DepthTexture = new RTexture<double>(Height, Width);


	bSSAA = false;
	bShadows = true;
	FOV = 90.0 * PI / 180.0;
	RayDepth = 1;
	ShadowSamples = 16;
	BackgroundColor = Vector3(0.0, 0.0, 0.0);
	AmbientLight = Vector3(0.07);
}


double RScene::GetPixel1D(uint16_t X, uint16_t Y, const SceneType1D Type) const
{	
	switch (Type)
	{
	case SceneType1D::DEPTH:
		return DepthTexture->Get(X, Y);
	case SceneType1D::METALLIC:
		return MetallicTexture->Get(X, Y);
	case SceneType1D::ROUGHNESS:
		return RoughnessTexture->Get(X, Y);
	case SceneType1D::TRANSMISSION:
		return TransmissionTexture->Get(X, Y);
	default:
		return 0.0;
	}
}

Vector3 RScene::GetPixel3D(uint16_t X, uint16_t Y, const SceneType3D Type) const
{
	X = Clamp<uint16_t>(X, 0, Width - 1);
	Y = Clamp<uint16_t>(Y, 0, Height - 1);

	const uint32_t Index = Y * Width + X;

	switch (Type)
	{
	case SceneType3D::HDR:
		return SceneTexture->Get(X, Y);
	case SceneType3D::BASE_COLOR:
		return BaseColorTexture->Get(X, Y);
	case SceneType3D::EMISSIVE:
		return EmissiveTexture->Get(X, Y);
	case SceneType3D::NORMAL:
		return NormalTexture->Get(X, Y);
	default:
		return Vector3(0.0);
	}
}

RTexture<double> RScene::GetTexture1D(const SceneType1D Type) const
{
	switch (Type)
	{
	case SceneType1D::DEPTH:
		return *DepthTexture;
	case SceneType1D::METALLIC:
		return *MetallicTexture;
	case SceneType1D::ROUGHNESS:
		return *RoughnessTexture;
	case SceneType1D::TRANSMISSION:
		return *TransmissionTexture;
	default:
		return RTexture<double>(0, 0);
	}
}

void RScene::GetTexture1D(const SceneType1D Type, RTexture<double>& OutTexture) const
{
	switch (Type)
	{
	case SceneType1D::DEPTH:
		OutTexture = *DepthTexture;
		break;
	case SceneType1D::METALLIC:
		OutTexture = *MetallicTexture;
		break;
	case SceneType1D::ROUGHNESS:
		OutTexture = *RoughnessTexture;
		break;
	case SceneType1D::TRANSMISSION:
		OutTexture = *TransmissionTexture;
		break;
	}
}


RTexture<Vector3> RScene::GetTexture3D(const SceneType3D Type) const
{
	switch (Type)
	{
	case SceneType3D::HDR:
		return *SceneTexture;
	case SceneType3D::BASE_COLOR:
		return *BaseColorTexture;
	case SceneType3D::EMISSIVE:
		return *EmissiveTexture;
	case SceneType3D::NORMAL:
		return *NormalTexture;
	default:
		return RTexture<Vector3>(0, 0);
	}
}

void RScene::GetTexture3D(const SceneType3D Type, RTexture<Vector3>& OutTexture) const
{
	switch (Type)
	{
	case SceneType3D::HDR:
		OutTexture = *SceneTexture;
		break;
	case SceneType3D::BASE_COLOR:
		OutTexture = *BaseColorTexture;
		break;
	case SceneType3D::EMISSIVE:
		OutTexture = *EmissiveTexture;
		break;
	case SceneType3D::NORMAL:
		OutTexture = *NormalTexture;
		break;
	default:
		OutTexture = RTexture<Vector3>(0, 0);
	}
}


void RScene::AddObject(OObject* Object)
{
	SceneObjects.insert(Object);
}

Vector3 RScene::SampleEnvMap(const Vector3& Direction) const
{
	Vector2 Polar = CartesianToPolar(Vector2(Direction.X, Direction.Y));
	const double U = ((Polar.Y / PI) + 1.0) * 0.5;
	const double V = (Direction.Z + 1.0) * 0.5;
	return EnvironmentTexture->GetByUV({ U, 1.0 - V }, true);
}

void RScene::ExtractLightSources()
{
	SceneLights.clear();
	auto InitialSceneObjects = SceneObjects;
	for(auto Object : InitialSceneObjects)
	{
		auto Light = dynamic_cast<OLight*>(Object);
		if (Light)
		{
			SceneLights.insert(Light);
			SceneObjects.erase(Light);
		}
	}
}

bool RScene::QueryScene(const RRay& Ray, RHit& OutHit) const
{
	double MinDist = INFINITY;
	bool bHit = false;

	for (auto* Object : SceneObjects)
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
}

void RScene::Render()
{
	const auto StartTime = std::chrono::high_resolution_clock::now();

	ExtractLightSources();
	
	const double AspectRatio = static_cast<double>(Width) / Height;

	double JitterMatrix[4 * 2] = {
		-1.0 / 4.0,  3.0 / 4.0,
		3.0 / 4.0,  1.0 / 3.0,
		-3.0 / 4.0, -1.0 / 4.0,
		1.0 / 4.0, -3.0 / 4.0
	};

	const Vector3 CameraPosition(0.0, 0.0, 0.0);
	//Vector3 CameraDirection(1.0, 0.0, 0.0);

	uint32_t CurrentPixel = 0;
	
	#pragma omp parallel for
	for (int32_t i = 0; i < Height; i++)
	{
		for (int32_t j = 0; j < Width; j++)
		{
			CurrentPixel++;
			PixelInfo Pixel;
			if (bSSAA)
			{				
				for (int32_t Sample = 0; Sample < 4; Sample++)
				{
					double SSX = 2.0 * (j + JitterMatrix[2 * Sample])		/ static_cast<double>(Width) - 1;
					double SSY = 2.0 * (i + JitterMatrix[2 * Sample + 1])	/ static_cast<double>(Height) - 1;
					SSX *= AspectRatio;

					const double PixelCameraX = SSX * tan(FOV / 2.0);
					const double PixelCameraY = SSY * tan(FOV / 2.0);

					RRay Ray;
					Ray.Origin = CameraPosition;
					Ray.Direction = Vector3(PixelCameraX, PixelCameraY, -1.0).Normalized();

					Pixel = Pixel + TraceRay(Ray);
				}
				Pixel = Pixel / 4.0;
			}
			else
			{

				double SSX = 2.0 * (j + 0.5) / (double)Width - 1;
				double SSY = 2.0 * (i + 0.5) / (double)Height - 1;
				SSX *= AspectRatio;

				const double PixelCameraX = SSX * tan(FOV / 2.0);
				const double PixelCameraY = SSY * tan(FOV / 2.0);

				RRay Ray;
				Ray.Origin = CameraPosition;
				Ray.Direction = Vector3(1.0, PixelCameraX, -PixelCameraY).Normalized();

				Pixel = TraceRay(Ray);
			}

			SceneTexture->Write(Pixel.ColorHDR, j, i);
			DepthTexture->Write(Pixel.Depth, j, i);
			MetallicTexture->Write(Pixel.Metallic, j, i);
			RoughnessTexture->Write(Pixel.Roughness, j, i);
			NormalTexture->Write(Pixel.Normal, j, i);
			TransmissionTexture->Write(Pixel.Transmission, j, i);
			BaseColorTexture->Write(Pixel.BaseColor, j, i);
			EmissiveTexture->Write(Pixel.Emissive, j, i);

			DrawPercent("Scene", "Rendering", CurrentPixel, Height * Width, 5);
		}
	}

	const auto EndTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> DeltaTime = EndTime - StartTime;
	const double Time = DeltaTime.count() / 1000.0;
	LOG("Scene", LogType::LOG, "Rendering Time: %.2f seconds", Time);
}

void RScene::SetEnvironmentTexture(const RTexture<Vector3>& Texture)
{
	EnvironmentTexture = new RTexture(Texture);
}

Vector3 RScene::TraceRay(const RRay& Ray, const uint8_t Depth) const
{
	//if (Depth >= RayDepth) return SampleEnvMap(Ray.Direction);

	//RHit HitInfo;

	///* If any object is hit by ray*/
	//if (QueryScene(Ray, HitInfo))
	//{
	//	const auto M = HitInfo.Mat;
	//	Vector3 Color(0.0);
	//	const bool Inside = (Ray.Direction | HitInfo.Normal) > 0;

	//	/* Loop over each Light Source */
	//	for (auto const* LightSource : SceneLights)
	//	{
	//		const auto LightPos = LightSource->Transform.GetPosition();
	//		const auto LightDir = (LightPos - HitInfo.Position).Normalized();

	//		const double ConeAngle = GetConeAngleByFittingSphere(LightSource->Radius,
	//			(LightPos - HitInfo.Position).Length());

	//		double LightAmount = 1.0;

	//		if (bShadows)
	//		{
	//			double SampleSum = 0.0;

	//			/* Collect samples for soft shadows */
	//			for (size_t Sample = 0; Sample < ShadowSamples; Sample++)
	//			{
	//				RRay ShadowRay;
	//				ShadowRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
	//				ShadowRay.Direction = GetRandomUnitVectorInsideCone(LightDir, ConeAngle);

	//				/*
	//				 * We don't want full shadows from transparent objects,
	//				 * so loop if the shadow ray hit object with transmission > 0.0
	//				 */
	//				double LightingScale = 1.0;
	//				RHit ShadowHit;
	//				while (QueryScene(ShadowRay, ShadowHit))
	//				{
	//					RHit TempHit;
	//					LightSource->Intersects(ShadowRay, TempHit);

	//					/* There's shadow only when the intersection point lies before LightSource */
	//					if (ShadowHit.Depth < TempHit.Depth)
	//					{
	//						if (ShadowHit.Mat.Transmission > 0.0)
	//						{
	//							/* Avoid self-intersection */
	//							const double Dot = ShadowHit.Normal | ShadowRay.Direction;
	//							ShadowRay.Origin = ShadowHit.Position + ShadowHit.Normal * (Dot < 0.0 ? -1e-6 : 1e-6);

	//							LightingScale *= ShadowHit.Mat.Transmission;
	//						}
	//						else
	//						{
	//							LightingScale = 0.0;
	//							break;
	//						}
	//					}
	//					else
	//					{
	//						break;
	//					}
	//				}

	//				SampleSum += std::sqrt(LightingScale) * Clamp(HitInfo.Normal | ShadowRay.Direction, 0.0, 1.0);
	//			}

	//			LightAmount = SampleSum / ShadowSamples;
	//		}


	//		const double Distance = (LightPos - HitInfo.Position).Length();
	//		const double Attenuation = 1.0 / (Distance * Distance);
	//		const Vector3 Radiance = LightSource->Color * Attenuation;

	//		double F = 0.0;
	//		const double rS = BRDF::BRDF(HitInfo.Normal, -Ray.Direction, LightDir, M.Roughness, F);
	//		const double kS = F;
	//		const Vector3 kD = (Vector3(1.0) - Vector3(kS)) / PI * LightAmount;

	//		RRay ReflectionRay;
	//		ReflectionRay.Origin = HitInfo.Position + HitInfo.Normal * (Inside ? -1e-6 : 1e-6);
	//		ReflectionRay.Direction = Ray.Direction.MirrorByVector(HitInfo.Normal);
	//		const Vector3 ReflectedLight = TraceRay(ReflectionRay, Depth + 1);


	//		Color = Color + (kD * M.Color * (1.0 - M.Metallic) + Vector3(rS)) * Radiance * (1.0 - M.Transmission); //Base light
	//		Color = Color + ReflectedLight * kS; // Reflected light			
	//		Color = Color * (Vector3(1.0) + AmbientLight);
	//	}

	//	
	//	if (M.Transmission > 0.0)
	//	{
	//		Vector3 TransmittedLight;
	//		RRay TransmittedRay;
	//		TransmittedRay.Direction = Refract(Ray.Direction, HitInfo.Normal, M.RefractiveIndex).Normalized();
	//		TransmittedRay.Origin = HitInfo.Position + HitInfo.Normal * (Inside ? 1e-6 : -1e-6);
	//		TransmittedLight = TraceRay(TransmittedRay, Depth + 1);

	//		Color = Color + TransmittedLight * M.Transmission; // Transmitted light
	//	}

	//	return Color + M.Emissive;
	//}

	return SampleEnvMap(Ray.Direction);
}

RScene::PixelInfo RScene::TraceRay(const RRay& Ray) const
{

	RHit HitInfo;

	/* If any object is hit by ray*/
	if (QueryScene(Ray, HitInfo))
	{
		
		const auto M = HitInfo.Mat;
		Vector3 Color(0.0);

		//const bool Inside = (Ray.Direction | HitInfo.Normal) > 0;

		///* Loop over each Light Source */
		//for (auto const* LightSource : SceneLights)
		//{
		//	const auto LightPos = LightSource->Transform.GetPosition();
		//	const auto LightDir = (LightPos - HitInfo.Position).Normalized();

		//	const double ConeAngle = GetConeAngleByFittingSphere(LightSource->Radius,
		//		(LightPos - HitInfo.Position).Length());
		//	
		//	double LightAmount = 1.0;

		//	if (bShadows)
		//	{
		//		double SampleSum = 0.0;

		//		/* Collect samples for soft shadows */
		//		for (size_t Sample = 0; Sample < ShadowSamples; Sample++)
		//		{
		//			RRay ShadowRay;
		//			ShadowRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
		//			ShadowRay.Direction = GetRandomUnitVectorInsideCone(LightDir, ConeAngle);

		//			/*
		//			 * We don't want full shadows from transparent objects,
		//			 * so loop if the shadow ray hit object with transmission > 0.0
		//			 */
		//			double LightingScale = 1.0;
		//			RHit ShadowHit;
		//			while (QueryScene(ShadowRay, ShadowHit))
		//			{
		//				RHit TempHit;
		//				LightSource->Intersects(ShadowRay, TempHit);

		//				/* There's shadow only when the intersection point lies before LightSource */
		//				if (ShadowHit.Depth < TempHit.Depth)
		//				{
		//					if (ShadowHit.Mat.Transmission > 0.0)
		//					{
		//						/* Avoid self-intersection */
		//						const double Dot = ShadowHit.Normal | ShadowRay.Direction;
		//						ShadowRay.Origin = ShadowHit.Position + ShadowHit.Normal * (Dot < 0.0 ? -1e-6 : 1e-6);

		//						LightingScale *= ShadowHit.Mat.Transmission;
		//					}
		//					else
		//					{
		//						LightingScale = 0.0;
		//						break;
		//					}
		//				}
		//				else
		//				{
		//					break;
		//				}
		//			}

		//			SampleSum += std::sqrt(LightingScale) * Clamp(HitInfo.Normal | ShadowRay.Direction, 0.0, 1.0);
		//		}

		//		LightAmount = SampleSum / ShadowSamples;
		//	}
		//	else
		//	{
		//		LightAmount *= Clamp(HitInfo.Normal | LightDir, 0.0, 1.0);
		//	}


		//	const double Distance = (LightPos - HitInfo.Position).Length();
		//	const double Attenuation = 1.0 / (Distance * Distance);
		//	const Vector3 Radiance = LightSource->Color * Attenuation;

		//	double F = 0.0;
		//	const double rS = BRDF::BRDF(HitInfo.Normal, -Ray.Direction, LightDir, M.Roughness, F);
		//	const double kS = F;
		//	const Vector3 kD = (Vector3(1.0) - Vector3(kS)) / PI * LightAmount;

		//	RRay ReflectionRay;
		//	ReflectionRay.Origin = HitInfo.Position + HitInfo.Normal * (Inside ? -1e-6 : 1e-6);
		//	ReflectionRay.Direction = Ray.Direction.MirrorByVector(HitInfo.Normal);
		//	const Vector3 ReflectedLight = TraceRay(ReflectionRay, 1);

		//	


		//	Color = Color + (kD * M.Color * (1.0 - M.Metallic) + Vector3(rS)) * Radiance * (1.0 - M.Transmission); //Base light
		//	Color = Color + ReflectedLight * kS; // Reflected light
		//	
		//	Color = Color * (Vector3(1.0) + AmbientLight);
		//}

		//
		//if (M.Transmission > 0.0)
		//{
		//	Vector3 TransmittedLight;
		//	RRay TransmittedRay;
		//	TransmittedRay.Direction = Refract(Ray.Direction, HitInfo.Normal, M.RefractiveIndex).Normalized();

		//	if (!TransmittedRay.Direction.NearlyZero())
		//	{
		//		TransmittedRay.Origin = HitInfo.Position + HitInfo.Normal * (Inside ? 1e-6 : -1e-6);
		//		TransmittedLight = TraceRay(TransmittedRay, 1);
		//		Color = Color + TransmittedLight * M.Transmission; // Transmitted light
		//	}

		//	
		//}

		
		for (auto* Light : SceneLights)
		{
			RLightInfo LightInfo(HitInfo, -Ray.Direction, Light);
			Color = Color + Shader->Light(LightInfo);
		}

		PixelInfo Pixel;
		//Pixel.Transmission	= M.Transmission;
		Pixel.Depth				= HitInfo.Depth;
		//Pixel.Emissive		= M.Emissive;
		//Pixel.Metallic		= M.Metallic;
		//Pixel.Roughness		= M.Roughness;
		//Pixel.BaseColor		= M.Color;
		Pixel.Normal		    = HitInfo.Normal;
		Pixel.ColorHDR			= Color;
		return Pixel;
		

		


	}

	return PixelInfo(SampleEnvMap(Ray.Direction));
}

void RScene::SetShader(ShadingModel* InShader)
{
	Shader = InShader;
}

Vector3 RLightInfo::GetLightVector() const
{
	return (Light->Transform.GetPosition() - Hit.Position).Normalized();
}

Vector3 RLightInfo::GetLightPosition() const
{
	return Light->Transform.GetPosition();
}

Vector3 RLightInfo::GetLightColor() const
{
	return Light->Color;
}
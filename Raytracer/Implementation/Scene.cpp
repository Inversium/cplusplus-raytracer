#include "../Headers/Scene.h"
#include "../Headers/CoreUtilities.h"
#include "../Headers/OObject.h"

double Texture1D::Get(const uint32_t X, const uint32_t Y) const
{
	const auto ClampedX = Clamp<uint32_t>(X, 0, GetWidth() - 1);
	const auto ClampedY = Clamp<uint32_t>(Y, 0, GetHeight() - 1);

	return Texture[ClampedY * Width + ClampedX];
}
double Texture1D::GetByUV(const Vector2 UV, bool UseBilinear) const
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	if (UseBilinear)
	{
		uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
		uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
		double ColorA = LInterp(Get(X, Y), Get(X + 1, Y), ClampedUV.X * Width - X);
		double ColorB = LInterp(Get(X, Y + 1), Get(X + 1, Y + 1), ClampedUV.X * Width - X);
		return LInterp(ColorA, ColorB, ClampedUV.Y * Height - Y);
	}
	else
	{
		uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
		uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
		return Get(X, Y);
	}
}
void Texture1D::Write(const double Value, const uint32_t X, const uint32_t Y)
{
	Texture[Y * Width + X] = Value;
}
void Texture1D::WriteByUV(const double Value, const Vector2 UV)
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
	uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
	Texture[Y * Width + X] = Value;
}


Vector3 Texture3D::Get(const uint32_t X, const uint32_t Y) const
{
	const auto ClampedX = Clamp<uint32_t>(X, 0, GetWidth() - 1);
	const auto ClampedY = Clamp<uint32_t>(Y, 0, GetHeight() - 1);

	return Texture[ClampedY * Width + ClampedX];
}
Vector3 Texture3D::GetByUV(const Vector2 UV, bool UseBilinear) const
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	if (UseBilinear)
	{
		uint16_t X = static_cast<uint16_t>(std::floor(ClampedUV.X * Width));
		uint16_t Y = static_cast<uint16_t>(std::floor(ClampedUV.Y * Height));
		Vector3 ColorA = LInterp(Get(X, Y), Get(X + 1, Y), ClampedUV.X * Width - X);
		Vector3 ColorB = LInterp(Get(X, Y + 1), Get(X + 1, Y + 1), ClampedUV.X * Width - X);
		return LInterp(ColorA, ColorB, ClampedUV.Y * Height - Y);
	}
	else
	{
		uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
		uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height); 
		return Get(X, Y);
	}
}
void Texture3D::Write(const Vector3& Value, const uint32_t X, const uint32_t Y)
{
	Texture[Y * Width + X] = Value;
}
void Texture3D::WriteByUV(const Vector3& Value, const Vector2 UV)
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
	uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
	Texture[Y * Width + X] = Value;
}

void Texture3D::Resize(uint16_t NewHeight, uint16_t NewWidth, bool UseBilinear)
{
	Texture3D NewTexture(NewHeight, NewWidth);

	#pragma omp parallel for
	for(uint16_t Y = 0; Y < NewHeight; Y++)
		for (uint16_t X = 0; X < NewWidth; X++)
		{
			double U = static_cast<double>(X) / NewWidth;
			double V = static_cast<double>(Y) / NewHeight;
			NewTexture.Write(GetByUV(Vector2(U, V), UseBilinear), X, Y);
		}

	*this = NewTexture;
}



Scene::Scene(const uint16_t InHeight, const uint16_t InWidth) : Height(InHeight), Width(InWidth)
{
	const uint32_t Size = Height * Width;
	SceneTexture = new Texture3D(Height, Width);		
	NormalTexture = new Texture3D(Height, Width);
	BaseColorTexture = new Texture3D(Height, Width);
	EmissiveTexture = new Texture3D(Height, Width);

	RoughnessTexture = new Texture1D(Height, Width);
	MetallicTexture = new Texture1D(Height, Width);
	TransmissionTexture = new Texture1D(Height, Width);
	DepthTexture = new Texture1D(Height, Width);


	bSSAA = false;
	FOV = 90.0 * PI / 180.0;
	RayDepth = 1;
	ShadowSamples = 16;
	BackgroundColor = Vector3(0.1, 0.2, 0.43);
	AmbientLight = Vector3(0.07);
}


double Scene::GetPixel1D(uint16_t X, uint16_t Y, const SceneType1D Type) const
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

Vector3 Scene::GetPixel3D(uint16_t X, uint16_t Y, const SceneType3D Type) const
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

Texture1D Scene::GetTexture1D(const SceneType1D Type) const
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
		return Texture1D(0, 0);
	}
}

void Scene::GetTexture1D(const SceneType1D Type, Texture1D& OutTexture) const
{
	switch (Type)
	{
	case SceneType1D::DEPTH:
		OutTexture = *DepthTexture;
	case SceneType1D::METALLIC:
		OutTexture = *MetallicTexture;
	case SceneType1D::ROUGHNESS:
		OutTexture = *RoughnessTexture;
	case SceneType1D::TRANSMISSION:
		OutTexture = *TransmissionTexture;
	}
}


Texture3D Scene::GetTexture3D(const SceneType3D Type) const
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
		return Texture3D(0, 0);
	}
}

void Scene::GetTexture3D(const SceneType3D Type, Texture3D& OutTexture) const
{
	switch (Type)
	{
	case SceneType3D::HDR:
		OutTexture = *SceneTexture;
	case SceneType3D::BASE_COLOR:
		OutTexture = *BaseColorTexture;
	case SceneType3D::EMISSIVE:
		OutTexture = *EmissiveTexture;
	case SceneType3D::NORMAL:
		OutTexture = *NormalTexture;
	default:
		OutTexture = Texture3D(0, 0);
	}
}


void Scene::AddObject(OObject* Object)
{
	SceneObjects.push_back(Object);
}

void Scene::ExtractLightSources()
{
	SceneLights.clear();
	for(auto* Object : SceneObjects)
	{
		auto* Light = dynamic_cast<OLight*>(Object);
		if (Light) SceneLights.push_back(Light);
	}
}

bool Scene::QueryScene(const RRay& Ray, RHit& OutHit) const
{
	double MinDist = LONG_MAX;
	bool bHit = false;

	for (auto* Object : SceneObjects)
	{
		RHit TempHit;
		if (Object->Intersects(Ray, TempHit))
		{
			bHit = true;
			if (TempHit.Depth < MinDist)
			{
				MinDist = TempHit.Depth;
				OutHit = TempHit;
			}
		}
	}
	return bHit;
}

void Scene::Render()
{
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
	for (size_t i = 0; i < Height; i++)
	{
		for (size_t j = 0; j < Width; j++)
		{
			CurrentPixel++;
			PixelInfo Pixel;
			if (bSSAA)
			{				
				for (size_t Sample = 0; Sample < 4; Sample++)
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
				Ray.Direction = Vector3(PixelCameraX, PixelCameraY, -1.0).Normalized();

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

			DrawPercent(CurrentPixel, Height * Width, 1);
		}
	}
}

Vector3 Scene::TraceRay(const RRay& Ray, const uint8_t Depth) const
{
	if (Depth >= RayDepth) return BackgroundColor;

	RHit HitInfo;

	/* If any object is hit by ray*/
	if (QueryScene(Ray, HitInfo))
	{
		const auto M = HitInfo.Mat;
		Vector3 Color(0.0);

		/* Loop over each Light Source */
		for (auto const* LightSource : SceneLights)
		{
			const Vector3 LightDir = (LightSource->Position - HitInfo.Position).Normalized();

			const double ConeAngle = GetConeAngleByFittingSphere(LightSource->Radius,
				(LightSource->Position - HitInfo.Position).Length());
			double SampleSum = 0.0;

			/* Collect samples for soft shadows */
			for (size_t Sample = 0; Sample < ShadowSamples; Sample++)
			{
				RRay ShadowRay;
				ShadowRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
				ShadowRay.Direction = GetRandomUnitVectorInsideCone(LightDir, ConeAngle);

				/*
				 * We don't want full shadows from transparent objects,
				 * so loop if the shadow ray hit object with transmission > 0.0
				 */
				double LightingScale = 1.0;
				RHit ShadowHit;
				while (QueryScene(ShadowRay, ShadowHit))
				{
					RHit TempHit;
					LightSource->Intersects(ShadowRay, TempHit);

					/* There's shadow only when the intersection point lies before LightSource */
					if (ShadowHit.Depth < TempHit.Depth)
					{
						if (ShadowHit.Mat.Transmission > 0.0)
						{
							/* Avoid self-intersection */
							const double Dot = ShadowHit.Normal | ShadowRay.Direction;
							ShadowRay.Origin = ShadowHit.Position + ShadowHit.Normal * (Dot < 0.0 ? -1e-6 : 1e-6);

							LightingScale *= ShadowHit.Mat.Transmission;
						}
						else
						{
							LightingScale = 0.0;
							break;
						}
					}
					else
					{
						break;
					}
				}

				SampleSum += std::sqrt(LightingScale) * Clamp(HitInfo.Normal | ShadowRay.Direction, 0.0, 1.0);
			}

			double LightAmount = SampleSum / ShadowSamples;


			const double Distance = (LightSource->Position - HitInfo.Position).Length();
			const double Attenuation = 1.0 / (Distance * Distance);
			const Vector3 Radiance = LightSource->Color * Attenuation;

			double F = 0.0;
			const double rS = BRDF::BRDF(HitInfo.Normal, -Ray.Direction, LightDir, M.Roughness, F);
			const double kS = F;
			const Vector3 kD = (Vector3(1.0) - Vector3(kS)) / PI * LightAmount;

			RRay ReflectionRay;
			ReflectionRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
			ReflectionRay.Direction = Ray.Direction.MirrorByVector(HitInfo.Normal);
			const Vector3 ReflectedLight = TraceRay(ReflectionRay, Depth + 1);

			Vector3 TransmittedLight;
			if (M.Transmission > 0.0)
			{
				RRay TransmittedRay;
				TransmittedRay.Origin = HitInfo.Position + HitInfo.Normal * -1e-6;
				TransmittedRay.Direction = Refract(Ray.Direction, HitInfo.Normal, M.RefractiveIndex);
				TransmittedLight = TraceRay(TransmittedRay, Depth + 1);
			}
			else
			{
				TransmittedLight = Vector3(0.0);
			}


			Color = Color + (kD * M.Color * (1.0 - M.Metallic) * (1.0 - M.Transmission) + Vector3(rS)) * Radiance; //Base light
			Color = Color + ReflectedLight * kS; // Reflected light
			Color = Color + TransmittedLight * M.Transmission; // Transmitted light
			Color = Color * (Vector3(1.0) + AmbientLight);
		}

		return (Color + M.Emissive).Clamp(0.0, 1.0);
	}

	return BackgroundColor;
}

Scene::PixelInfo Scene::TraceRay(const RRay& Ray) const
{

	RHit HitInfo;

	/* If any object is hit by ray*/
	if (QueryScene(Ray, HitInfo))
	{
		const auto M = HitInfo.Mat;
		Vector3 Color(0.0);

		/* Loop over each Light Source */
		for (auto const* LightSource : SceneLights)
		{
			const Vector3 LightDir = (LightSource->Position - HitInfo.Position).Normalized();

			const double ConeAngle = GetConeAngleByFittingSphere(LightSource->Radius,
				(LightSource->Position - HitInfo.Position).Length());
			double SampleSum = 0.0;

			/* Collect samples for soft shadows */
			for (size_t Sample = 0; Sample < ShadowSamples; Sample++)
			{
				RRay ShadowRay;
				ShadowRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
				ShadowRay.Direction = GetRandomUnitVectorInsideCone(LightDir, ConeAngle);

				/*
				 * We don't want full shadows from transparent objects,
				 * so loop if the shadow ray hit object with transmission > 0.0
				 */
				double LightingScale = 1.0;
				RHit ShadowHit;
				while (QueryScene(ShadowRay, ShadowHit))
				{
					RHit TempHit;
					LightSource->Intersects(ShadowRay, TempHit);

					/* There's shadow only when the intersection point lies before LightSource */
					if (ShadowHit.Depth < TempHit.Depth)
					{
						if (ShadowHit.Mat.Transmission > 0.0)
						{
							/* Avoid self-intersection */
							const double Dot = ShadowHit.Normal | ShadowRay.Direction;
							ShadowRay.Origin = ShadowHit.Position + ShadowHit.Normal * (Dot < 0.0 ? -1e-6 : 1e-6);

							LightingScale *= ShadowHit.Mat.Transmission;
						}
						else
						{
							LightingScale = 0.0;
							break;
						}
					}
					else
					{
						break;
					}
				}

				SampleSum += std::sqrt(LightingScale) * Clamp(HitInfo.Normal | ShadowRay.Direction, 0.0, 1.0);
			}

			double LightAmount = SampleSum / ShadowSamples;


			const double Distance = (LightSource->Position - HitInfo.Position).Length();
			const double Attenuation = 1.0 / (Distance * Distance);
			const Vector3 Radiance = LightSource->Color * Attenuation;

			double F = 0.0;
			const double rS = BRDF::BRDF(HitInfo.Normal, -Ray.Direction, LightDir, M.Roughness, F);
			const double kS = F;
			const Vector3 kD = (Vector3(1.0) - Vector3(kS)) / PI * LightAmount;

			RRay ReflectionRay;
			ReflectionRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
			ReflectionRay.Direction = Ray.Direction.MirrorByVector(HitInfo.Normal);
			const Vector3 ReflectedLight = TraceRay(ReflectionRay, 1);

			Vector3 TransmittedLight;
			if (M.Transmission > 0.0)
			{
				RRay TransmittedRay;
				TransmittedRay.Origin = HitInfo.Position + HitInfo.Normal * -1e-6;
				TransmittedRay.Direction = Refract(Ray.Direction, HitInfo.Normal, M.RefractiveIndex);
				TransmittedLight = TraceRay(TransmittedRay, 1);
			}
			else
			{
				TransmittedLight = Vector3(0.0);
			}


			Color = Color + (kD * M.Color * (1.0 - M.Metallic) * (1.0 - M.Transmission) + Vector3(rS)) * Radiance; //Base light
			Color = Color + ReflectedLight * kS; // Reflected light
			Color = Color + TransmittedLight * M.Transmission; // Transmitted light
			Color = Color * (Vector3(1.0) + AmbientLight);
		}

		PixelInfo Pixel;
		Pixel.Transmission	= M.Transmission;
		Pixel.Depth			= HitInfo.Depth;
		Pixel.Emissive		= M.Emissive;
		Pixel.Metallic		= M.Metallic;
		Pixel.Roughness		= M.Roughness;
		Pixel.BaseColor		= M.Color;
		Pixel.Normal		= HitInfo.Normal;
		Pixel.ColorHDR		= Color + M.Emissive;
		return Pixel;
	}

	return PixelInfo();
}
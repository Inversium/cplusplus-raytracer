#include "../Headers/Shader.h"
#include "../Headers/Scene.h"
#include "../Headers/math/Math.h"
#include "../Headers/Core.h"
#include "../Headers/OObject.h"
#include "../Headers/ShadingModel.h"
#include "../Headers/Material.h"
#include "../Headers/Light.h"


RShader::RShader()
{
	BackgroundColor = Vector3(0.0);
	bIndirectSampling = true;
	bDirectSampling = false;
	bShadows = true;
	bTranslucency = true;
	SamplesIndirect = 4;
	SamplesDirect = 64;
	RayDepth = 1;
}

Vector3 RShader::DirectLighting(const RScene* const Scene, const RRay& Ray, const RHit& Hit) const
{
	Vector3 FinalColor(0.0);
	for (auto& Light : Scene->SceneLights)
	{
		if (bDirectSampling)
		{
			Vector3 SampledLight(0.0);
			for (size_t i = 0; i < SamplesDirect; i++)
			{
				const Vector3 View = -Ray.Direction;
				const Vector3 LightDir = Light->SampleDirection(Hit.Position);
				RHit LightHit;
				RRay LightRay(Hit.Position, LightDir);
				Light->Intersects(LightRay, LightHit);

				if (bShadows)
				{
					/* If the point is in shadow, skip direct lighting for this Light */
					RRay ShadowRay;
					ShadowRay.Origin = Hit.Position + Hit.Normal * 1e-6;
					ShadowRay.Direction = LightDir;
					RHit ShadowHit;
					if (Scene->QueryScene(ShadowRay, ShadowHit) && ShadowHit.Depth < LightHit.Depth && ShadowHit.Object != Light) continue;
				}

				const double NdotL = std::max(LightDir | Hit.Normal, 0.0);

				const double Attenuation = Light->Area() / (LightHit.Depth * LightHit.Depth);
				const double LightWeight = Attenuation * std::max(-LightDir | LightHit.Normal, 0.0);
				const Vector3 Radiance = Light->GetColor() * LightWeight;

				RLightInfo LightInfo(Hit.Normal, View, LightDir, Hit.Mat);

				SampledLight += Scene->ModelBRDF->Get(LightInfo).Diffuse * Radiance * NdotL;
			}
			FinalColor += SampledLight / SamplesDirect;
		}
		else
		{
			const Vector3 View = -Ray.Direction;
			const Vector3 LightDir = (Light->Transform.GetPosition() - Hit.Position).Normalized();
			const double LightDist = (Light->Transform.GetPosition() - Hit.Position).Length();

			if (bShadows)
			{
				/* If the point is in shadow, skip direct lighting for this Light */
				RRay ShadowRay;
				ShadowRay.Origin = Hit.Position + Hit.Normal * 1e-6;
				ShadowRay.Direction = LightDir;
				RHit ShadowHit;
				if (Scene->QueryScene(ShadowRay, ShadowHit) && ShadowHit.Depth < LightDist && ShadowHit.Object != Light) continue;
			}

			const double NdotL = std::max(LightDir | Hit.Normal, 0.0);

			const double Attenuation = Light->Area() / (LightDist * LightDist);
			const Vector3 Radiance = Light->GetColor() * Attenuation;

			RLightInfo LightInfo(Hit.Normal, View, LightDir, Hit.Mat);

			FinalColor += Scene->ModelBRDF->Get(LightInfo).Color * Radiance * NdotL;
		}
	}

	return FinalColor;
}

Vector3 RShader::Light(const RScene* const Scene, const RRay& Ray) const
{
	return LightInternal(Scene, Ray);
}

Vector3 RShader::LightInternal(const RScene* const Scene, const RRay& Ray) const
{
	Vector3 FinalColor(0.0);

	RHit Hit;
	// Return color from environment map if we didn't hit anything
	if (!Scene->QueryScene(Ray, Hit)) return Scene->SampleEnvMap(Ray.Direction);

	// We don't want to collect light for the light source, so we'll return just the light's emissive color
	if (Hit.Mat->GetMaterialType() == MaterialType::Light)
	{
		Vector3 Emissive;
		Hit.Mat->GetVectorProperty(Emissive, "Emissive");
		return Emissive;
	}

	/* Direct lighting */
	FinalColor += DirectLighting(Scene, Ray, Hit);


	
	/* Indirect lighting */
	if (bIndirectSampling)
	{
		Vector3 IndirectLighting(0.0);

		for (size_t i = 0; i < SamplesIndirect; i++)
		{	
			// Importance sample the brdf
			RLightInfo LightInfo(Hit.Normal, -Ray.Direction, Vector3(0.0), Hit.Mat);
			const Vector3 Micronormal = Scene->ModelBRDF->Sample(LightInfo);

			// Create a new ray from generated micronormal
			RRay NewRay;
			NewRay.Direction = Ray.Direction.MirrorByVector(Micronormal);
			NewRay.Origin = Hit.Position + Hit.Normal * 1e-6;

			const double NdotL = std::max(0.0, (Micronormal | NewRay.Direction));

			LightInfo.Light = NewRay.Direction;

			IndirectLighting += Scene->ModelBRDF->Get(LightInfo).Color * RayRecurse(Scene, NewRay, 0) * NdotL / Scene->ModelBRDF->GetPDF(LightInfo);
		}
		IndirectLighting /= static_cast<double>(SamplesIndirect);

		FinalColor += IndirectLighting;
	}

	
	return FinalColor;
}

Vector3 RShader::RayRecurse(const RScene* const Scene, const RRay& Ray, const uint8_t Depth) const
{
	if (Depth >= RayDepth) return BackgroundColor;

	RHit Hit;
	// Return color from environment map if we didn't hit anything
	if (!Scene->QueryScene(Ray, Hit)) return Scene->SampleEnvMap(Ray.Direction); 

	// We don't want to collect light for the light source, so we'll return just the light's emissive color
	if (Hit.Mat->GetMaterialType() == MaterialType::Light)
	{
		Vector3 Emissive;
		Hit.Mat->GetVectorProperty(Emissive, "Emissive");
		return Vector3(0.0);
	}

	// Importance sample the brdf
	RLightInfo LightInfo(Hit.Normal, -Ray.Direction, Vector3(0.0), Hit.Mat);
	const Vector3 Micronormal = Scene->ModelBRDF->Sample(LightInfo);

	// Create a new ray from generated micronormal
	RRay NewRay;
	NewRay.Direction = Ray.Direction.MirrorByVector(Micronormal);
	NewRay.Origin = Hit.Position + Hit.Normal * 1e-6;

	const double NdotL = std::max(0.0, (Micronormal | NewRay.Direction));
	

	LightInfo.Light = NewRay.Direction;

	return Scene->ModelBRDF->Get(LightInfo).Color * RayRecurse(Scene, NewRay, Depth + 1) * NdotL / (Scene->ModelBRDF->GetPDF(LightInfo)) + DirectLighting(Scene, Ray, Hit);
}

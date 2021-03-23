#include "../Headers/Shader.h"
#include "../Headers/Scene.h"
#include "../Headers/math/Math.h"
#include "../Headers/Core.h"
#include "../Headers/OObject.h"
#include "../Headers/ShadingModel.h"
#include "../Headers/Material.h"


RShader::RShader()
{
	BackgroundColor = Vector3(0.0);
	bGlobalIllumination = true;
	bShadows = true;
	SamplesGI = 256;
	RayDepth = 1;
}

Vector3 RShader::Light(const RScene* const Scene, const RRay& Ray) const
{
	return LightInternal(Scene, Ray, 0);
}

Vector3 RShader::LightInternal(const RScene* const Scene, const RRay& Ray, const uint8_t Depth) const
{
	if (Depth > RayDepth) return BackgroundColor;

	RHit Hit;
	if (!Scene->QueryScene(Ray, Hit)) return Scene->SampleEnvMap(Ray.Direction);

	Vector3 FinalColor(0.0);

	for (auto* Light : Scene->SceneLights)
	{
		const Vector3 View = -Ray.Direction;
		RLightInfo LightInfo(Hit, View, Light);


		/* If the point is in shadow, skip direct lighting for this Light */
		RRay ShadowRay;
		ShadowRay.Origin = Hit.Position + Hit.Normal * 1e-6;
		ShadowRay.Direction = (Light->Transform.GetPosition() - Hit.Position).Normalized();
		RHit ShadowHit;
		if (Scene->QueryScene(ShadowRay, ShadowHit) && ShadowHit.Depth < LightInfo.GetLightDistance()) continue;

		FinalColor += Scene->ModelBRDF->Get(LightInfo).Color;
	}

	
	if (bGlobalIllumination)
	{
		Vector3 IndirectLighting(0.0);

		for (size_t i = 0; i < SamplesGI; i++)
		{
			RRay NewRay;
			NewRay.Direction = GetRandomVectorHemisphere(Hit.Normal);
			//NewRay.Direction = Hit.Normal;
			NewRay.Origin = Hit.Position + Hit.Normal * 1e-6;
			
			const double Dot = std::max(0.0, NewRay.Direction | Hit.Normal);

			IndirectLighting += LightInternal(Scene, NewRay, Depth + 1) * Dot;
		}
		IndirectLighting /= static_cast<double>(SamplesGI) * PI;

		FinalColor += IndirectLighting * Hit.Mat->Color;
	}
	
	return FinalColor;
}

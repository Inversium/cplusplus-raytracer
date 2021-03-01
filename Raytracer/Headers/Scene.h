#pragma once
#include <cassert>
#include <cstdint>
#include <vector>
#include <set>

#include "Core.h"
#include "Texture.h"

class OLight;
class OObject;
class ShadingModel;


class RScene
{
public:
	RScene(const uint16_t InHeight, const uint16_t InWidth);

	enum class SceneType1D : uint8_t
	{
		DEPTH			= 0,
		ROUGHNESS		= 1,
		TRANSMISSION	= 2,
		METALLIC		= 3
	};

	enum class SceneType3D : uint8_t
	{
		HDR			= 0,
		NORMAL		= 1,
		BASE_COLOR	= 2,
		EMISSIVE	= 3
	};

	struct PixelInfo
	{
		PixelInfo() {};
		PixelInfo(const Vector3& HDRColor) : ColorHDR(HDRColor) {}

		double Depth = 0.0;
		double Transmission = 0.0;
		double Metallic = 0.0;
		double Roughness = 0.0;
		Vector3 BaseColor = Vector3(0.0);
		Vector3 Normal = Vector3(0.0);
		Vector3 Emissive = Vector3(0.0);
		Vector3 ColorHDR = Vector3(0.0);

		PixelInfo operator+(const PixelInfo& Other) const
		{
			PixelInfo Pixel;
			Pixel.Transmission	= this->Transmission + Other.Transmission;
			Pixel.Depth			= this->Depth + Other.Depth;
			Pixel.Emissive		= this->Emissive + Other.Emissive;
			Pixel.Metallic		= this->Metallic + Other.Metallic;
			Pixel.Roughness		= this->Roughness + Other.Roughness;
			Pixel.BaseColor		= this->BaseColor + Other.BaseColor;
			Pixel.Normal		= this->Normal + Other.Normal;
			Pixel.ColorHDR		= this->ColorHDR + Other.ColorHDR;
			return Pixel;
		}

		PixelInfo operator/(const double Scale) const
		{
			PixelInfo Pixel;
			Pixel.Transmission	= this->Transmission / Scale;
			Pixel.Depth			= this->Depth / Scale;
			Pixel.Emissive		= this->Emissive / Scale;
			Pixel.Metallic		= this->Metallic / Scale;
			Pixel.Roughness		= this->Roughness / Scale;
			Pixel.BaseColor		= this->BaseColor / Scale;
			Pixel.Normal		= this->Normal / Scale;
			Pixel.ColorHDR		= this->ColorHDR / Scale;
			return Pixel;
		}
	};

	bool bSSAA;
	bool bShadows;
	uint8_t RayDepth;
	uint16_t ShadowSamples;
	Vector3 BackgroundColor;
	Vector3 AmbientLight;
	double FOV;

	const uint16_t Height;
	const uint16_t Width;

private:	
	
	/* HDR output of the scene render */
	RTexture<Vector3>* SceneTexture = nullptr;

	/* Depth Render of the Scene */
	RTexture<double>* DepthTexture = nullptr;

	/* Roughness Render of the Scene */
	RTexture<double>* RoughnessTexture = nullptr;

	/* Transmission Render of the Scene */
	RTexture<double>* TransmissionTexture = nullptr;

	/* Metallic Render of the Scene */
	RTexture<double>* MetallicTexture = nullptr;

	/* Normal render of the scene */
	RTexture<Vector3>* NormalTexture = nullptr;

	/* Emissive only render of the scene */
	RTexture<Vector3>* EmissiveTexture = nullptr;

	/* Base Color only render of the scene */
	RTexture<Vector3>* BaseColorTexture = nullptr;

	/* Container with all scene objects */
	std::set<OObject*> SceneObjects;

	/* Container with all scene lights */
	std::set<OLight*> SceneLights;

	RTexture<Vector3>* EnvironmentTexture = nullptr;

	ShadingModel* Shader = nullptr;


public:

	double GetPixel1D(uint16_t X, uint16_t Y, const SceneType1D Type) const;
	Vector3 GetPixel3D(uint16_t X, uint16_t Y, const SceneType3D Type) const;

	void GetTexture1D(const SceneType1D Type, RTexture<double>& OutTexture) const;
	RTexture<double> GetTexture1D(const SceneType1D Type) const;
	
	void GetTexture3D(const SceneType3D Type, RTexture<Vector3>& OutTexture) const;
	RTexture<Vector3> GetTexture3D(const SceneType3D Type) const;

	void AddObject(OObject* Object);
	
	void Render();

	void SetEnvironmentTexture(const RTexture<Vector3>& Texture);

	bool QueryScene(const RRay& Ray, RHit& OutHit) const;

	void SetShader(ShadingModel* InShader);

private:

	Vector3 SampleEnvMap(const Vector3& Direction) const;
	void ExtractLightSources();
	
	PixelInfo TraceRay(const RRay& Ray) const;
	Vector3 TraceRay(const RRay& Ray, const uint8_t Depth) const;

};

struct RLightInfo
{
	RHit Hit;
	Vector3 View;
private:
	OLight* Light = nullptr;

public:
	RLightInfo(const RHit& InHit, const Vector3& InView, OLight* InLight) : Hit(InHit), View(InView), Light(InLight) {}

public:
	Vector3 GetLightVector() const;

	Vector3 GetLightPosition() const;

	Vector3 GetLightColor() const;
};
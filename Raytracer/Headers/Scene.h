#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

//#include "CoreUtilities.h"
#include "math/mvector.h"

class OLight;
struct RHit;
struct RRay;
class OObject;

struct Texture1D
{
	Texture1D(const uint32_t InHeight, const uint32_t InWidth) : Height(InHeight), Width(InWidth)
	{
		Texture.resize(Height * Width);
	}
	

private:
	std::vector<double> Texture;
	uint32_t Height;
	uint32_t Width;

public:
	uint32_t GetHeight() const { return Height; }
	uint32_t GetWidth() const { return Width; }
	double Get(const uint32_t X, const uint32_t Y) const;
	double GetByUV(const Vector2 UV, bool UseBilinear = false) const;
	void Write(const double Value, const uint32_t X, const uint32_t Y);
	void WriteByUV(const double Value, const Vector2 UV);
	std::vector<double>::iterator Begin() { return Texture.begin(); }
	std::vector<double>::iterator End() { return Texture.end(); }
};
struct Texture3D
{
	Texture3D(const uint32_t InHeight, const uint32_t InWidth) : Height(InHeight), Width(InWidth)
	{
		Texture.resize(Height * Width);
	}
	

private:
	std::vector<Vector3> Texture;
	uint32_t Height;
	uint32_t Width;

public:
	uint32_t GetHeight() const { return Height; }
	uint32_t GetWidth() const { return Width; }
	Vector3 Get(const uint32_t X, const uint32_t Y) const;
	Vector3 GetByUV(const Vector2 UV, bool UseBilinear = false) const;
	void Write(const Vector3& Value, const uint32_t X, const uint32_t Y);
	void WriteByUV(const Vector3& Value, const Vector2 UV);
	std::vector<Vector3>::iterator Begin() { return Texture.begin(); }
	std::vector<Vector3>::iterator End() { return Texture.end(); }
	void Resize(uint16_t NewHeight, uint16_t NewWidth, bool UseBilinear = false);
};

class Scene
{
public:
	Scene(const uint16_t InHeight, const uint16_t InWidth);

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
	uint8_t RayDepth;
	uint16_t ShadowSamples;
	Vector3 BackgroundColor;
	Vector3 AmbientLight;
	double FOV;

	const uint16_t Height;
	const uint16_t Width;

private:	
	
	/* HDR output of the scene render */
	Texture3D* SceneTexture = nullptr;

	/* Depth Render of the Scene */
	Texture1D* DepthTexture = nullptr;

	/* Roughness Render of the Scene */
	Texture1D* RoughnessTexture = nullptr;

	/* Transmission Render of the Scene */
	Texture1D* TransmissionTexture = nullptr;

	/* Metallic Render of the Scene */
	Texture1D* MetallicTexture = nullptr;

	/* Normal render of the scene */
	Texture3D* NormalTexture = nullptr;

	/* Emissive only render of the scene */
	Texture3D* EmissiveTexture = nullptr;

	/* Base Color only render of the scene */
	Texture3D* BaseColorTexture = nullptr;

	/* Container with all scene objects */
	std::vector<OObject*> SceneObjects;

	/* Container with all scene objects */
	std::vector<OLight*> SceneLights;

public:

	double GetPixel1D(uint16_t X, uint16_t Y, const SceneType1D Type) const;
	Vector3 GetPixel3D(uint16_t X, uint16_t Y, const SceneType3D Type) const;

	void GetTexture1D(const SceneType1D Type, Texture1D& OutTexture) const;
	Texture1D GetTexture1D(const SceneType1D Type) const;
	
	void GetTexture3D(const SceneType3D Type, Texture3D& OutTexture) const;
	Texture3D GetTexture3D(const SceneType3D Type) const;

	void AddObject(OObject* Object);
	
	void Render();

private:

	void ExtractLightSources();
	bool QueryScene(const RRay& Ray, RHit& OutHit) const;
	PixelInfo TraceRay(const RRay& Ray) const;
	Vector3 TraceRay(const RRay& Ray, const uint8_t Depth) const;
};


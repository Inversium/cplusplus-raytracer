#pragma once
#include <unordered_map>

#include "math/Vector.h"

enum class MaterialType
{
	None,
	BlinnPhong,
	PBR,
	Light
};


struct RMaterial
{
private:
	std::unordered_map<const char*, Vector3> VectorProperties;
	std::unordered_map<const char*, double> FloatProperties;
	MaterialType Type = MaterialType::None;

public:
	RMaterial() {};
	RMaterial(MaterialType InType) : Type(InType) {};

public:
	void AddVectorProperty(const char* Key, const Vector3& Value);
	void AddFloatProperty(const char* Key, const double Value);
	bool GetVectorProperty(Vector3& OutValue, const char* Key) const;
	bool GetFloatProperty(double& OutValue, const char* Key) const;
	MaterialType GetMaterialType() const { return Type; }

	void InitializeBlinnPhong(const Vector3& InColor, const double InSpecularExponent);
	void InitializePBR(const Vector3& InColor, const Vector3 InEmissive, const double InRoughness, const double InMetallic, const double InRefractiveIndex, const double InTransmission);
	void InitializeLight(const Vector3& InEmissive);

	static RMaterial Metal(const double Roughness);
	static RMaterial Glass();
	static RMaterial Mirror();
	static RMaterial RedDielectric(const double Roughness);
	static RMaterial WhiteDielectric(const double Roughness);
};

inline void RMaterial::AddVectorProperty(const char* Key, const Vector3& Value)
{
	VectorProperties.insert({ Key, Value });
}

inline void RMaterial::AddFloatProperty(const char* Key, const double Value)
{
	FloatProperties.insert({ Key, Value });
}

inline bool RMaterial::GetVectorProperty(Vector3& OutValue, const char* Key) const
{
	std::unordered_map<const char*, Vector3>::const_iterator Iter;
	if ((Iter = VectorProperties.find(Key)) != VectorProperties.end())
	{
		OutValue = Iter->second;
		return true;
	}
	return false;
}

inline bool RMaterial::GetFloatProperty(double& OutValue, const char* Key) const
{
	std::unordered_map<const char*, double>::const_iterator Iter;
	if ((Iter = FloatProperties.find(Key)) != FloatProperties.end())
	{
		OutValue = Iter->second;
		return true;
	}
	return false;
}

inline void RMaterial::InitializeBlinnPhong(const Vector3& InColor, const double InSpecularExponent)
{
	Type = MaterialType::BlinnPhong;
	VectorProperties.clear();
	FloatProperties.clear();

	AddVectorProperty("Color", InColor);
	AddFloatProperty("SpecularExponent", InSpecularExponent);
}

inline void RMaterial::InitializePBR(const Vector3& InColor, const Vector3 InEmissive, const double InRoughness, const double InMetallic, const double InRefractiveIndex, const double InTransmission)
{
	Type = MaterialType::PBR;
	VectorProperties.clear();
	FloatProperties.clear();

	AddVectorProperty("Color", InColor);
	AddVectorProperty("Emissive", InEmissive);
	AddFloatProperty("Roughness", InRoughness);
	AddFloatProperty("Metallic", InMetallic);
	AddFloatProperty("RefractiveIndex", InRefractiveIndex);
	AddFloatProperty("Transmission", InTransmission);
}

inline void RMaterial::InitializeLight(const Vector3& InEmissive)
{
	Type = MaterialType::Light;
	VectorProperties.clear();
	FloatProperties.clear();

	AddVectorProperty("Emissive", InEmissive);
}

inline RMaterial RMaterial::Metal(const double Roughness)
{
	static RMaterial OutMaterial;
	OutMaterial.InitializePBR(Vector3(1.0), Vector3(0.0), Roughness, 1.0, 1.0, 0.0);
	return OutMaterial;
}

inline RMaterial RMaterial::Glass()
{
	static RMaterial OutMaterial;
	OutMaterial.InitializePBR(Vector3(0.0), Vector3(0.0), 0.0, 0.0, 1.4, 1.0);
	return OutMaterial;
}

inline RMaterial RMaterial::Mirror()
{
	static RMaterial OutMaterial;
	OutMaterial.InitializePBR(Vector3(1.0), Vector3(0.0), 0.0, 1.0, 1.0, 0.0);
	return OutMaterial;
}

inline RMaterial RMaterial::WhiteDielectric(const double Roughness)
{
	static RMaterial OutMaterial;
	OutMaterial.InitializePBR(Vector3(1.0), Vector3(0.0), Roughness, 0.0, 1.0, 0.0);
	return OutMaterial;
}

inline RMaterial RMaterial::RedDielectric(const double Roughness)
{
	static RMaterial OutMaterial;
	OutMaterial.InitializePBR(Vector3(1.0, 0.0, 0.0), Vector3(0.0), Roughness, 0.0, 1.0, 0.0);
	return OutMaterial;
}


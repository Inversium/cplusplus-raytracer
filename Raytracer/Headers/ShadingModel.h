#pragma once

#include "math/Vector.h"

class RScene;

struct BRDFResult
{
	BRDFResult(const Vector3 InColor, const double InF0) : Color(InColor), F0(InF0) {}

	Vector3 Color;
	double F0;
};

class BRDF
{

public:
	virtual BRDFResult Get(const class RLightInfo& LightInfo) const = 0;
private:
	virtual class RMaterial* GetDefaultMaterial() const = 0;
};


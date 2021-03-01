#pragma once

#include <functional>

class Vector3;
class RScene;


class ShadingModel
{

public:
	virtual Vector3 Light(const RLightInfo& LightInfo, std::function<bool(const RRay&, RHit&)> QueryScene = nullptr) const = 0;
private:
	virtual class RMaterial* GetDefaultMaterial() const = 0;
};
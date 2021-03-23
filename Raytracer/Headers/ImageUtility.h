#pragma once

#include "math/Vector.h"

template<typename T>
class RTexture;


enum class EImageFormat
{
	JPG,
	PNG,
	BMP
};

class ImageUtility
{
public:
	static bool LoadImage(RTexture<Vector3>& Image, const char* Filename);
	static void SaveImage(const RTexture<Vector3>& Image, const char* Filename, const EImageFormat Format = EImageFormat::PNG);
};

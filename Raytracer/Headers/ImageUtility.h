#pragma once

#include "math/Vector.h"


class RTexture;


enum class EImageFormat
{
	JPG,
	PNG,
	BMP
};

namespace ImageUtility
{
	bool LoadImage(RTexture* Image, const char* Filename);
	void SaveImage(const RTexture* const Image, const char* Filename, const EImageFormat Format = EImageFormat::PNG, bool bUseDate = false);
};

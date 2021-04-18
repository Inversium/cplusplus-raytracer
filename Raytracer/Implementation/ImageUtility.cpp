#include "../Headers/ImageUtility.h"
#include "../Headers/Texture.h"
#include "../Headers/Core.h"
#include <cstdint>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../ThirdParty/stb_image_write.h"


bool ImageUtility::LoadImage(RTexture* Image, const char* Filename)
{
	int32_t ImageHeight, ImageWidth, Ch;
	float* ImagePtr = stbi_loadf(Filename, &ImageWidth, &ImageHeight, &Ch, 0);
	if (!ImagePtr || Ch != 3) return false;

	Image->Initialize(ImageHeight, ImageWidth);

	for (uint32_t i = 0; i < ImageWidth * ImageHeight; i++)
	{
		RColor Color;
		Color.R = ImagePtr[i * 3 + 0];
		Color.G = ImagePtr[i * 3 + 1];
		Color.B = ImagePtr[i * 3 + 2];
		Image->Write(Color, i % ImageWidth, i / ImageWidth);
	}

	stbi_image_free(ImagePtr);
	return true;
}

void ImageUtility::SaveImage(const RTexture* const Image, const char* Filename, const EImageFormat Format)
{
	std::string FilenameStr(Filename);
	
	std::filesystem::create_directory(std::filesystem::path(Filename).parent_path());

	auto Height = Image->GetHeight();
	auto Width = Image->GetWidth();

	auto Data = MakeUnique<uint8_t>(Height * Width * 3);

	for (size_t i = 0; i < Height; i++)
	{
		for (size_t j = 0; j < Width; j++)
		{
			const auto Pixel = Image->Get(j, i);
			Data.get()[Width * i * 3 + j * 3]	= static_cast<uint8_t>(Clamp<uint8_t>(Pixel.R * 255, 0, 255));
			Data.get()[Width * i * 3 + j * 3 + 1] = static_cast<uint8_t>(Clamp<uint8_t>(Pixel.G * 255, 0, 255));
			Data.get()[Width * i * 3 + j * 3 + 2] = static_cast<uint8_t>(Clamp<uint8_t>(Pixel.B * 255, 0, 255));
		}
	}

	switch (Format)
	{
	case EImageFormat::PNG:
		if (!FilenameStr.ends_with(".png")) FilenameStr += ".png";
		stbi_write_png(FilenameStr.c_str(), Width, Height, 3, (void*)Data.get(), 0);
		break;
	case EImageFormat::JPG:
		if (!FilenameStr.ends_with(".jpg")) FilenameStr += ".jpg";
		stbi_write_jpg(FilenameStr.c_str(), Width, Height, 3, (void*)Data.get(), 100);
		break;
	case EImageFormat::BMP:
		if (!FilenameStr.ends_with(".jpg")) FilenameStr += ".bmp";
		stbi_write_bmp(FilenameStr.c_str(), Width, Height, 3, (void*)Data.get());
		break;
	}
	
}
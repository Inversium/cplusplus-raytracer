#include "../Headers/ImageUtility.h"
#include "../Headers/Texture.h"
#include "../Headers/Core.h"
#include <cstdint>
#include <filesystem>
#include <chrono>
#include <format>

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

void ImageUtility::SaveImage(const RTexture* const Image, const char* Filename, const EImageFormat Format, bool bUseDate)
{
	std::string FilenameStr(Filename);
	
	std::filesystem::create_directory(std::filesystem::path(Filename).parent_path());

	auto Height = Image->GetHeight();
	auto Width = Image->GetWidth();

	auto Data = MakeUnique<uint8_t[]>(Height * Width * 3);

	for (size_t i = 0; i < Height; i++)
	{
		for (size_t j = 0; j < Width; j++)
		{
			const auto Pixel = Image->Get(j, i);
			Data.get()[Width * i * 3 + j * 3]	  = Clamp<uint8_t>(Pixel.R * 255, 0, 255);
			Data.get()[Width * i * 3 + j * 3 + 1] = Clamp<uint8_t>(Pixel.G * 255, 0, 255);
			Data.get()[Width * i * 3 + j * 3 + 2] = Clamp<uint8_t>(Pixel.B * 255, 0, 255);
		}
	}

	if (bUseDate)
	{
		using namespace std::chrono;
		const auto Time = current_zone()->to_local(system_clock::now());
		std::string FormattedTime = std::format("_{:%F_%T}", Time);
		FilenameStr += FormattedTime;
	}

	auto LogImage = [&FilenameStr, &Filename](int Res)
	{
		if (Res != 0)
		{
			LOG("Image Saving", LogType::ERROR, "Couldn't save image {}, function returned {}", FilenameStr, Res);
		}
		else
		{
			auto AbsPath = std::filesystem::absolute(std::filesystem::path(Filename)).generic_string();
			LOG("Image Saving", LogType::LOG, "Successfully saved image {} at {}", std::filesystem::path(FilenameStr).filename().generic_string(), AbsPath);
		}
	};

	switch (Format)
	{
	case EImageFormat::PNG:
		{
			if (!FilenameStr.ends_with(".png")) FilenameStr += ".png";
			int Res = stbi_write_png(FilenameStr.c_str(), Width, Height, 3, (void*)Data.get(), 0);
			LogImage(Res);
			break;
		}
	case EImageFormat::JPG:
		{
			if (!FilenameStr.ends_with(".jpg")) FilenameStr += ".jpg";
			int Res = stbi_write_jpg(FilenameStr.c_str(), Width, Height, 3, (void*)Data.get(), 100);
			LogImage(Res);
			break;
		}
	case EImageFormat::BMP:
		{
			if (!FilenameStr.ends_with(".bmp")) FilenameStr += ".bmp";
			int Res = stbi_write_bmp(FilenameStr.c_str(), Width, Height, 3, (void*)Data.get());
			LogImage(Res);
			break;
		}
	}
	
}
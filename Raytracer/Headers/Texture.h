#pragma once

#include <vector>


#include "Core.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "../ThirdParty/stb_image.h"


template<class T = Vector3>
struct RTexture
{
	RTexture(const uint32_t InHeight, const uint32_t InWidth) : Height(InHeight), Width(InWidth)
	{
		Texture.resize(static_cast<size_t>(Height) * Width);
	}


private:
	std::vector<T> Texture;
	uint32_t Height;
	uint32_t Width;

public:
	/* Get Height of the Texture in pixels */
	uint32_t GetHeight() const { return Height; }

	/* Get Width of the Texture in pixels */
	uint32_t GetWidth() const { return Width; }

	/* 
	 *  Get value of the pixel at (X, Y) coordinates assuming X is in range [0, Width] and Y in range [0, Height]
	 *  Values outside the range of height and width will be clamped 
	 */
	T Get(const uint32_t X, const uint32_t Y) const;

	/* 
	 *  Get value of the pixel at (X, Y) coordinates assuming X is in range [0, 1] and Y in range [0, 1]
	 *  Values outside the range of height and width will be wrapped to [0, 1]
	 *  With UseBilinear = true this will return interpolated value of 4 pixels around the given point, 
	 *  otherwise it'll return the nearest value.
	 */
	T GetByUV(const Vector2 UV, bool UseBilinear = false) const;

	/* 
	 *  Write value to the pixel at (X, Y) coordinates assuming X is in range [0, Width] and Y in range [0, Height]
	 *  Values outside the range of height and width won't be clamped and this will throw out of bounds exception
	 */
	void Write(const T& Value, const uint32_t X, const uint32_t Y);

	/*
	 *  Write value to the pixel at (X, Y) coordinates assuming X is in range [0, 1] and Y in range [0, 1]
	 *  Values outside the range of height and width will be wrapped to [0, 1]
	 */
	void WriteByUV(const T& Value, const Vector2 UV);

	/* Getters for iterators for texture manipulation */
	typename std::vector<T>::iterator Begin();
	typename std::vector<T>::iterator End();
	const T* Data() const;

	/*
	 * Resize texture to new resolution
	 * With UseBilinear = true it will interpolate new values,
	 * otherwise it'll just use nearest values.
	 */
	void Resize(uint16_t NewHeight, uint16_t NewWidth, bool UseBilinear = false);

	/* Load Texture from file */
	bool Load(const char* Filename);
};




template<class T>
inline T RTexture<T>::Get(const uint32_t X, const uint32_t Y) const
{
	const auto ClampedX = Clamp<uint32_t>(X, 0, GetWidth() - 1);
	const auto ClampedY = Clamp<uint32_t>(Y, 0, GetHeight() - 1);

	return Texture[ClampedY * Width + ClampedX];
}

template<class T>
inline T RTexture<T>::GetByUV(const Vector2 UV, bool UseBilinear) const
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	if (UseBilinear)
	{
		uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
		uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
		T ColorA = LInterp(Get(X, Y), Get(X + 1, Y), ClampedUV.X * Width - X);
		T ColorB = LInterp(Get(X, Y + 1), Get(X + 1, Y + 1), ClampedUV.X * Width - X);
		return LInterp(ColorA, ColorB, ClampedUV.Y * Height - Y);
	}
	else
	{
		uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
		uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
		return Get(X, Y);
	}
}

template<class T>
inline void RTexture<T>::Write(const T& Value, const uint32_t X, const uint32_t Y)
{
	Texture[Y * Width + X] = Value;
}

template<class T>
inline void RTexture<T>::WriteByUV(const T& Value, const Vector2 UV)
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
	uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
	Texture[Y * Width + X] = Value;
}

template<class T>
inline void RTexture<T>::Resize(uint16_t NewHeight, uint16_t NewWidth, bool UseBilinear)
{
	RTexture NewTexture(NewHeight, NewWidth);

#pragma omp parallel for collapse(2)
	for (int16_t Y = 0; Y < NewHeight; Y++)
		for (int16_t X = 0; X < NewWidth; X++)
		{
			double U = static_cast<double>(X) / NewWidth;
			double V = static_cast<double>(Y) / NewHeight;
			NewTexture.Write(GetByUV(Vector2(U, V), UseBilinear), X, Y);
		}

	*this = NewTexture;
}


template<class T>
inline bool RTexture<T>::Load(const char* Filename)
{
	int32_t ImageHeight, ImageWidth, Ch;
	float* Image = stbi_loadf(Filename, &ImageWidth, &ImageHeight, &Ch, 0);
	if (!Image || Ch != 3) return false;

	Height = ImageHeight;
	Width = ImageWidth;
	Texture.clear();
	Texture.resize(0);

	for (uint32_t i = 0; i < ImageWidth * ImageHeight; i++)
	{
		Vector3 Color;
		Color.X = Image[i * 3 + 0];
		Color.Y = Image[i * 3 + 1];
		Color.Z = Image[i * 3 + 2];
		Texture.push_back(Color);
	}

	stbi_image_free(Image);
	return true;
}

template<class T>
inline typename std::vector<T>::iterator RTexture<T>::Begin() { return Texture.begin(); }

template<class T>
inline typename std::vector<T>::iterator RTexture<T>::End() { return Texture.end(); }

template<class T>
inline const T* RTexture<T>::Data() const { return Texture.data(); }
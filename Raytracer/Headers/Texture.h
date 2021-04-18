#pragma once

#include <vector>


#include "math/Vector.h"
#include "CoreUtilities.h"
#include "ImageUtility.h"
#include "Color.h"

class RTexture
{
public:
	RTexture(const uint32_t InHeight, const uint32_t InWidth) : Height(InHeight), Width(InWidth)
	{
		Texture.resize(static_cast<size_t>(Height) * Width);
	}


private:
	std::vector<RColor> Texture;
	uint32_t Height;
	uint32_t Width;

public:
	/* Get Height of the Texture in pixels */
	uint32_t GetHeight() const { return Height; }

	/* Get Width of the Texture in pixels */
	uint32_t GetWidth() const { return Width; }

	/*
	 *  Get value of the pixel at (X, Y) coordinates assuming X is in range [0, Width) and Y in range [0, Height)
	 *  Values outside the range of height and width will be clamped
	 */
	RColor Get(const uint32_t X, const uint32_t Y) const;

	/*
	 *  Get value of the pixel at (X, Y) coordinates assuming X is in range [0, 1] and Y in range [0, 1]
	 *  Values outside the range of height and width will be wrapped to [0, 1]
	 *  With UseBilinear = true this will return interpolated value of 4 pixels around the given point,
	 *  otherwise it'll return the nearest value.
	 */
	RColor GetByUV(const Vector2 UV, bool UseBilinear = false) const;

	/*
	 *  Write value to the pixel at (X, Y) coordinates assuming X is in range [0, Width) and Y in range [0, Height)
	 *  Values outside the range of height and width won't be clamped and this will throw out of bounds exception
	 */
	void Write(const RColor& Value, const uint32_t X, const uint32_t Y);

	/*
	 *  Write value to the pixel at (X, Y) coordinates assuming X is in range [0, 1] and Y in range [0, 1]
	 *  Values outside the range of height and width will be wrapped to [0, 1]
	 */
	void WriteByUV(const RColor& Value, const Vector2 UV);

	void Initialize(uint16_t NewHeight, uint16_t NewWidth);

	/* Getters for iterators for texture manipulation */
	typename std::vector<RColor>::iterator Begin();
	typename std::vector<RColor>::const_iterator Begin() const;
	typename std::vector<RColor>::iterator End();
	typename std::vector<RColor>::const_iterator End() const;
	const RColor* Data() const;

	/*
	 * Resize texture to new resolution
	 * With UseBilinear = true it will interpolate new values,
	 * otherwise it'll just use nearest values.
	 */
	void Resize(uint16_t NewHeight, uint16_t NewWidth, bool UseBilinear = false);

	/* Load Texture from file */
	bool Load(const char* Filename);
};





inline RColor RTexture::Get(const uint32_t X, const uint32_t Y) const
{
	const auto ClampedX = Clamp<uint32_t>(X, 0, GetWidth() - 1);
	const auto ClampedY = Clamp<uint32_t>(Y, 0, GetHeight() - 1);

	return Texture[static_cast<size_t>(ClampedY) * Width + ClampedX];
}


inline RColor RTexture::GetByUV(const Vector2 UV, bool UseBilinear) const
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	if (UseBilinear)
	{
		uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
		uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
		RColor ColorA = LInterp(Get(X, Y), Get(X + 1, Y), ClampedUV.X * Width - X);
		RColor ColorB = LInterp(Get(X, Y + 1), Get(X + 1, Y + 1), ClampedUV.X * Width - X);
		return LInterp(ColorA, ColorB, ClampedUV.Y * Height - Y);
	}
	else
	{
		uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
		uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
		return Get(X, Y);
	}
}


inline void RTexture::Write(const RColor& Value, const uint32_t X, const uint32_t Y)
{
	Texture[static_cast<size_t>(Y) * Width + X] = Value;
}


inline void RTexture::WriteByUV(const RColor& Value, const Vector2 UV)
{
	Vector2 ClampedUV(std::fmod(UV.X, 1.0), std::fmod(UV.Y, 1.0));
	uint16_t X = static_cast<uint16_t>(ClampedUV.X * Width);
	uint16_t Y = static_cast<uint16_t>(ClampedUV.Y * Height);
	Texture[static_cast<size_t>(Y) * Width + X] = Value;
}

inline void RTexture::Initialize(uint16_t NewHeight, uint16_t NewWidth)
{
	Texture.clear();
	Texture.resize(static_cast<size_t>(NewWidth) * NewHeight);
	Height = NewHeight;
	Width = NewWidth;
}


inline void RTexture::Resize(uint16_t NewHeight, uint16_t NewWidth, bool UseBilinear)
{
	RTexture NewTexture(NewHeight, NewWidth);

	#pragma omp parallel for
	for (int32_t Y = 0; Y < NewHeight; Y++)
		for (int32_t X = 0; X < NewWidth; X++)
		{
			double U = static_cast<double>(X) / NewWidth;
			double V = static_cast<double>(Y) / NewHeight;
			NewTexture.Write(GetByUV(Vector2(U, V), UseBilinear), X, Y);
		}

	*this = NewTexture;
}


inline bool RTexture::Load(const char* Filename)
{
	return ImageUtility::LoadImage(this, Filename);
}


inline typename std::vector<RColor>::iterator RTexture::Begin() { return Texture.begin(); }

inline typename std::vector<RColor>::const_iterator RTexture::Begin() const { return Texture.begin(); }


inline typename std::vector<RColor>::iterator RTexture::End() { return Texture.end(); }

inline typename std::vector<RColor>::const_iterator RTexture::End() const { return Texture.end(); }


inline const RColor* RTexture::Data() const { return Texture.data(); }
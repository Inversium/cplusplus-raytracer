#pragma once
#include <cmath>


#include "Scene.h"
#include <execution>
#include "math/Vector.h"

inline double Gaussian(const double X, const double Sigma = 1.0)
{
	const double A = Sigma * Sigma * 2.0 * PI;
	return A * std::exp(-(X * X) / (2.0 * Sigma * Sigma));
}

inline double Luminance(const Vector3& Color)
{
	return Vector3(0.3, 0.59, 0.11) | Color;
}

inline void GaussianBlur(RTexture<Vector3>& Texture, const double Sigma)
{
	const uint32_t TextureSize = Texture.GetHeight() * Texture.GetWidth();
	const int32_t Radius = 2 * static_cast<int32_t>(std::ceil(Sigma * 3)) + 1;
	const uint8_t Height = Texture.GetHeight();
	const uint8_t Width = Texture.GetWidth();
	RTexture<Vector3> InitialTexture = Texture;


	/* First pass of blurring - Horizontal */	
	#pragma omp parallel for
	for (int32_t X = 0; X < Texture.GetWidth(); X++)
		for (int32_t Y = 0; Y < Texture.GetHeight(); Y++) 
		{
			double KernelSum = 0.0;
			Vector3 Color(0.0);

			for (int32_t i = -Radius; i <= Radius; i++)
			{
				const double Gauss = Gaussian(i, 5.0);
				KernelSum += Gauss;
				Color = Color + Gauss * InitialTexture.Get(X + i, Y);
			}

			Color = Color / KernelSum;
			Texture.Write(Color, X, Y);
		}
	
	/* For second pass we need to use already blurred texture */
	InitialTexture = Texture;

	/* Second pass of blurring - Vertical */
	#pragma omp parallel for
	for (int32_t X = 0; X < Texture.GetWidth(); X++)
		for (int32_t Y = 0; Y < Texture.GetHeight(); Y++)
		{
			double KernelSum = 0.0;
			Vector3 Color(0.0);

			for (int32_t i = -Radius; i <= Radius; i++)
			{
				const double Gauss = Gaussian(i, 5.0);
				KernelSum += Gauss;
				Color = Color + Gauss * InitialTexture.Get(X, Y + i);
			}

			Color = Color / KernelSum;
			Texture.Write(Color, X, Y);
		}
}


inline void Bloom(RTexture<Vector3>& Texture, const double LuminanceThreshold = 3.0, const double Sigma = 3.0)
{
	auto BrightFilter = [LuminanceThreshold](const Vector3& Color)
	{
		if (Luminance(Color) > LuminanceThreshold) return Color;
		else return Vector3(0.0);
	};

	RTexture<Vector3> ExtractedBloom(Texture.GetHeight(), Texture.GetWidth());
	#pragma omp parallel for collapse(2)
	for (int32_t i = 0; i < Texture.GetHeight(); i++)
		for (int32_t j = 0; j < Texture.GetWidth(); j++)
		{
			const Vector3 Value = BrightFilter(Texture.Get(j, i));
			ExtractedBloom.Write(Value, j, i);
		}

	GaussianBlur(ExtractedBloom, Sigma);

	#pragma omp parallel for collapse(2)
	for (int32_t i = 0; i < Texture.GetHeight(); i++)
		for (int32_t j = 0; j < Texture.GetWidth(); j++)
			Texture.Write(Texture.Get(j, i) + ExtractedBloom.Get(j, i), j, i);
}

inline void GammaCorrection(RTexture<Vector3>& Texture, const double Gamma = 2.2)
{	
	std::transform(std::execution::par, Texture.Begin(), Texture.End(), Texture.Begin(), 
		[Gamma](const Vector3& Color)
		{
			return Vector3(
				std::pow(Color.X, Gamma),
				std::pow(Color.Y, Gamma),
				std::pow(Color.Z, Gamma)
			);
		});
}

inline void ToneCompression(RTexture<Vector3>& Texture, const double Exposure = 1.0)
{
	std::transform(std::execution::par, Texture.Begin(), Texture.End(), Texture.Begin(),
		[Exposure](const Vector3& Color)
		{
			return Vector3(
				1.0 - std::exp(-Color.X * Exposure),
				1.0 - std::exp(-Color.Y * Exposure),
				1.0 - std::exp(-Color.Z * Exposure)
			);
		});
}

inline void Sharpen(RTexture<Vector3>& Texture, const double Sharpness = 1.0)
{
	auto const InitialTexture(Texture);

	#pragma omp parallel for collapse(2)
	for(int32_t Y = 0; Y < Texture.GetHeight(); Y++)
		for (int32_t X = 0; X < Texture.GetWidth(); X++)
		{
			const Vector3 Sample = InitialTexture.Get(X, Y);
			auto Diffs = {
				Sample - InitialTexture.Get(X + 1, Y),
				Sample - InitialTexture.Get(X - 1, Y),
				Sample - InitialTexture.Get(X, Y + 1),
				Sample - InitialTexture.Get(X, Y - 1)
			};
			const Vector3 MaxDiff = *std::max_element(Diffs.begin(), Diffs.end());
			const Vector3 NewValue = Sample + Sharpness * MaxDiff;
			Texture.Write(NewValue, X, Y);			
		}
}
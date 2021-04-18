#pragma once
#include <cmath>
#include <execution>

#include "Core.h"
#include "Scene.h"
#include "math/Vector.h"

inline double Gaussian(const double X, const double Sigma = 1.0)
{
	const double A = Sigma * Sigma * 2.0 * PI;
	return A * std::exp(-(X * X) / (2.0 * Sigma * Sigma));
}


inline void GaussianBlur(UniquePtr<RTexture>& Texture, const double Sigma)
{
	const uint32_t TextureSize = Texture->GetHeight() * Texture->GetWidth();
	const int32_t Radius = 2 * static_cast<int32_t>(std::ceil(Sigma * 3)) + 1;
	const uint32_t Height = Texture->GetHeight();
	const uint32_t Width = Texture->GetWidth();
	auto InitialTexture(*Texture);


	/* First pass of blurring - Horizontal */	
	#pragma omp parallel for
	for (int32_t X = 0; X < Width; X++)
		for (int32_t Y = 0; Y < Height; Y++) 
		{
			double KernelSum = 0.0;
			RColor Color;

			for (int32_t i = -Radius; i <= Radius; i++)
			{
				const double Gauss = Gaussian(i, 5.0);
				KernelSum += Gauss;
				Color = Color + Gauss * InitialTexture.Get(X + i, Y);
			}

			Color = Color / KernelSum;
			Texture->Write(Color, X, Y);
		}
	
	/* For second pass we need to use already blurred texture */
	InitialTexture = *Texture;

	/* Second pass of blurring - Vertical */
	#pragma omp parallel for
	for (int32_t X = 0; X < Width; X++)
		for (int32_t Y = 0; Y < Height; Y++)
		{
			double KernelSum = 0.0;
			RColor Color;

			for (int32_t i = -Radius; i <= Radius; i++)
			{
				const double Gauss = Gaussian(i, 5.0);
				KernelSum += Gauss;
				Color = Color + Gauss * InitialTexture.Get(X, Y + i);
			}

			Color = Color / KernelSum;
			Texture->Write(Color, X, Y);
		}
}


inline void Bloom(UniquePtr<RTexture>& Texture, const double LuminanceThreshold = 3.0, const double Sigma = 3.0)
{
	auto BrightFilter = [LuminanceThreshold](const RColor& Color)
	{
		if (Color.Luminance() > LuminanceThreshold) return Color;
		else return RColor();
	};

	auto ExtractedBloom = MakeUnique<RTexture>(Texture->GetHeight(), Texture->GetWidth());
	#pragma omp parallel for
	for (int32_t i = 0; i < Texture->GetHeight(); i++)
		for (int32_t j = 0; j < Texture->GetWidth(); j++)
		{
			const RColor Value = BrightFilter(Texture->Get(j, i));
			ExtractedBloom->Write(Value, j, i);
		}

	GaussianBlur(ExtractedBloom, Sigma);

	#pragma omp parallel for
	for (int32_t i = 0; i < Texture->GetHeight(); i++)
		for (int32_t j = 0; j < Texture->GetWidth(); j++)
			Texture->Write(Texture->Get(j, i) + ExtractedBloom->Get(j, i), j, i);
}

inline void GammaCorrection(UniquePtr<RTexture>& Texture, const double Gamma = 2.2)
{	
	std::transform(std::execution::par, Texture->Begin(), Texture->End(), Texture->Begin(), 
		[Gamma](const RColor& Color)
		{
			return RColor(
				std::pow(Color.R, Gamma),
				std::pow(Color.G, Gamma),
				std::pow(Color.B, Gamma),
				Color.A
			);
		});
}

inline void ToneCompression(UniquePtr<RTexture>& Texture, const double Exposure = 1.0)
{
	std::transform(std::execution::par, Texture->Begin(), Texture->End(), Texture->Begin(),
		[Exposure](const RColor& Color)
		{
			return RColor(
				1.0 - std::exp(-Color.R * Exposure),
				1.0 - std::exp(-Color.G * Exposure),
				1.0 - std::exp(-Color.B * Exposure),
				Color.A
			);
		});
}

inline void Sharpen(UniquePtr<RTexture>& Texture, const double Sharpness = 1.0)
{
	auto const InitialTexture(*Texture);

	#pragma omp parallel for
	for(int32_t Y = 0; Y < Texture->GetHeight(); Y++)
		for (int32_t X = 0; X < Texture->GetWidth(); X++)
		{
			const RColor Sample = InitialTexture.Get(X, Y);
			auto Diffs = {
				Sample - InitialTexture.Get(X + 1, Y),
				Sample - InitialTexture.Get(X - 1, Y),
				Sample - InitialTexture.Get(X, Y + 1),
				Sample - InitialTexture.Get(X, Y - 1)
			};
			const RColor MaxDiff = *std::max_element(Diffs.begin(), Diffs.end());
			const RColor NewValue = Sample + Sharpness * MaxDiff;
			Texture->Write(NewValue, X, Y);			
		}
}
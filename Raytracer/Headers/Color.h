#pragma once
#include "math/Vector.h"



struct RColor
{
	double R;
	double G;
	double B;
	double A;

	RColor(const double InR, const double InG, const double InB, const double InA)
		: R(InR), G(InG), B(InB), A(InA) {}

	explicit RColor(const Vector3& InRGB, const double InA)
		: R(InRGB.X), G(InRGB.Y), B(InRGB.Z), A(InA) {}

	RColor(const Vector3& InRGB)
		: R(InRGB.X), G(InRGB.Y), B(InRGB.Z), A(0.0) {}

	explicit RColor() : R(0.0), G(0.0), B(0.0), A(0.0) {}

	Vector3 ToVector() const { return { R, G, B }; }

	double Luminance() const { return 0.3 * R + 0.59 * G + 0.11 * B; }


	RColor operator+(const RColor& Other) const;
	RColor operator-(const RColor& Other) const;
	RColor operator*(const RColor& Other) const;
	RColor operator/(const RColor& Other) const;

	RColor operator*(const double Scale) const;
	RColor operator/(const double Scale) const;

	RColor& operator+=(const RColor& Other);
	RColor& operator*=(const RColor& Other);
	RColor& operator/=(const RColor& Other);
	RColor& operator-=(const RColor& Other);
	RColor& operator*=(const double Scale);
	RColor& operator/=(const double Scale);

	bool operator==(const RColor& Other) const;
	bool operator!=(const RColor& Other) const;
	bool operator<(const RColor& Other) const;
	bool operator>(const RColor& Other) const;
	bool operator<=(const RColor& Other) const;
	bool operator>=(const RColor& Other) const;

};

inline RColor RColor::operator+(const RColor& Other) const
{
	return 
	{ 
		R + Other.R, 
		G + Other.G, 
		B + Other.B, 
		A + Other.A
	};
}

inline RColor RColor::operator-(const RColor& Other) const
{
	return
	{
		R - Other.R,
		G - Other.G,
		B - Other.B,
		A - Other.A
	};
}

inline RColor RColor::operator*(const RColor& Other) const
{
	return
	{
		R * Other.R,
		G * Other.G,
		B * Other.B,
		A * Other.A
	};
}

inline RColor RColor::operator/(const RColor& Other) const
{
	return
	{
		R / Other.R,
		G / Other.G,
		B / Other.B,
		A / Other.A
	};
}

inline RColor RColor::operator*(const double Scale) const
{
	return
	{
		R * Scale,
		G * Scale,
		B * Scale,
		A * Scale
	};
}

inline RColor RColor::operator/(const double Scale) const
{
	return
	{
		R / Scale,
		G / Scale,
		B / Scale,
		A / Scale
	};
}

inline RColor& RColor::operator+=(const RColor& Other)
{
	return *this = *this + Other;
}

inline RColor& RColor::operator-=(const RColor& Other)
{
	return *this = *this - Other;
}

inline RColor& RColor::operator*=(const RColor& Other)
{
	return *this = *this * Other;
}

inline RColor& RColor::operator/=(const RColor& Other)
{
	return *this = *this / Other;
}

inline RColor& RColor::operator*=(const double Scale)
{
	return *this = *this * Scale;
}

inline RColor& RColor::operator/=(const double Scale)
{
	return *this = *this / Scale;
}

inline bool RColor::operator==(const RColor& Other) const
{
	return R == Other.R && G == Other.G && B == Other.B && A == Other.A;
}

inline bool RColor::operator!=(const RColor& Other) const
{
	return !(*this == Other);
}

inline bool RColor::operator<(const RColor& Other) const
{
	return Luminance() < Other.Luminance();
}

inline bool RColor::operator>(const RColor& Other) const
{
	return Luminance() > Other.Luminance();
}

inline bool RColor::operator<=(const RColor& Other) const
{
	return Luminance() <= Other.Luminance();
}

inline bool RColor::operator>=(const RColor& Other) const
{
	return Luminance() >= Other.Luminance();
}

inline RColor operator*(const double Scale, const RColor& Color)
{
	return Color * Scale;
}
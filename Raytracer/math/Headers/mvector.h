#pragma once
#include <cmath>
#include <string>
#include <stdexcept>

#define PI 3.1415926535
#define SMALL_NUMBER 1e-7



struct Vector2
{
	double X, Y;

	Vector2() : X(0.0), Y(0.0) {}

	Vector2(const double InX, const double InY) : X(InX), Y(InY) {}

	explicit Vector2(const double InValue) : X(InValue), Y(InValue) {}

	//returns length of this vector
	double Length() const;

	//returns a normalized copy of an input vector
	Vector2 Normalized() const;

	//returns a human readable string for debugging
	std::string ToString() const;

	/*
	  if vector length greater the max param, set length of this vector to the max
	  if vector length lesser the min param, set length of this vector to the min
	  otherwise do nothing
	*/
	Vector2 Clamp(const double Min, const double Max);

	//returns true if all components of two vectors are equal within the given tolerance
	bool Equals(const Vector2& Other, const double Tolerance = 1e-6) const;

	//return true if vector length == 1
	bool IsNormalized() const;

	/*
	  mirror vector by an input normal
	  input normal is automatically Normalized
	*/
	Vector2 MirrorByVector(const Vector2& Normal) const;

	double operator[](const int Index) const;
	Vector2 operator-() const;

	Vector2 operator+(const Vector2& Other) const;
	Vector2 operator-(const Vector2& Other) const;
	Vector2 operator*(const Vector2& Other) const;
	Vector2 operator/(const Vector2& Other) const;
	double operator|(const Vector2& Other) const;

	Vector2 operator*(const double Scale) const;
	Vector2 operator/(const double Scale) const;

	bool operator==(const Vector2& Other) const;
	bool operator!=(const Vector2& Other) const;
	bool operator<(const Vector2& Other) const;
	bool operator>(const Vector2& Other) const;
	bool operator<=(const Vector2& Other) const;
	bool operator>=(const Vector2& Other) const;

};

struct Vector3
{

	double X, Y, Z;

	Vector3() : X(0.0), Y(0.0), Z(0.0) {}

	Vector3(const double InX, const double InY, const double InZ) : X(InX), Y(InY), Z(InZ) {}

	explicit Vector3(const double InValue) : X(InValue), Y(InValue), Z(InValue) {}

	explicit Vector3(const Vector2& V, const double InZ) : X(V.X), Y(V.Y), Z(InZ) {}

	//returns length of this vector
	double Length() const;

	//returns a normalized copy of an input vector
	Vector3 Normalized() const;

	//returns a human readable string for debugging
	std::string ToString() const;

	/*
	  if vector length greater the max param, set length of this vector to the max
	  if vector length lesser the min param, set length of this vector to the min
	  otherwise do nothing
	*/
	Vector3 Clamp(const double Min, const double Max);

	//returns true if all components of two vectors are equal within the given tolerance
	bool Equals(const Vector3& Other, const double Tolerance = 1e-6) const;

	//return true if vector length == 1
	bool IsNormalized() const;

	static double Angle(Vector3& v1, Vector3& v2);

	static Vector3 CrossProduct(Vector3& v1, Vector3& v2);

	/*
	  mirror vector by an input normal
	  input normal is automatically Normalized
	*/
	Vector3 MirrorByVector(const Vector3& Normal) const;



	double operator[](const int Index) const;
	Vector3 operator-() const;

	Vector3 operator+(const Vector3& Other) const;
	Vector3 operator-(const Vector3& Other) const;
	Vector3 operator*(const Vector3& Other) const;
	Vector3 operator/(const Vector3& Other) const;
	double operator|(const Vector3& Other) const;
	Vector3 operator^(const Vector3& Other) const;

	Vector3 operator*(const double Scale) const;
	Vector3 operator/(const double Scale) const;

	bool operator==(const Vector3& Other) const;
	bool operator!=(const Vector3& Other) const;
	bool operator<(const Vector3& Other) const;
	bool operator>(const Vector3& Other) const;
	bool operator<=(const Vector3& Other) const;
	bool operator>=(const Vector3& Other) const;
};


inline double Vector3::Length() const
{
	return sqrt(X * X + Y * Y + Z * Z);
}
inline double Vector2::Length() const
{
	return sqrt(X * X + Y * Y);
}

inline Vector3 Vector3::Normalized() const
{
	const double L = Length();
	return { X / L, Y / L, Z / L };
}
inline Vector2 Vector2::Normalized() const
{
	const double L = Length();
	return { X / L, Y / L };
}

inline std::string Vector3::ToString() const
{
	std::string Out;
	Out += "X = " + std::to_string(X);
	Out += " Y = " + std::to_string(Y);
	Out += " Z = " + std::to_string(Z);
	return Out;
}
inline std::string Vector2::ToString() const
{
	std::string Out;
	Out += "X = " + std::to_string(X);
	Out += " Y = " + std::to_string(Y);
	return Out;
}

inline Vector3 Vector3::Clamp(const double Min, const double Max)
{
	if (Length() > Max)
	{
		*this = this->Normalized() * Max;
		return *this;
	}
	else if (Length() < Min)
	{
		*this = this->Normalized() * Min;
		return *this;
	}
	return *this;
}
inline Vector2 Vector2::Clamp(const double Min, const double Max)
{
	if (Length() > Max)
	{
		*this = this->Normalized() * Max;
		return *this;
	}
	else if (Length() < Min)
	{
		*this = this->Normalized() * Min;
		return *this;
	}
	return *this;
}

inline bool Vector3::Equals(const Vector3& Other, const double Tolerance) const
{
	return std::abs(Other.X - X) <= Tolerance && std::abs(Other.Y - Y) <= Tolerance && std::abs(Other.Y - Y) <= Tolerance;
}
inline bool Vector2::Equals(const Vector2& Other, const double Tolerance) const
{
	return std::abs(Other.X - X) <= Tolerance && std::abs(Other.Y - Y) <= Tolerance;
}

inline bool Vector3::IsNormalized() const
{
	return std::abs(1 - Length()) < SMALL_NUMBER;
}
inline bool Vector2::IsNormalized() const
{
	return std::abs(1 - Length()) < SMALL_NUMBER;
}

inline Vector3 Vector3::MirrorByVector(const Vector3& Normal) const
{
	return *this - Normal * (2 * (*this | Normal));
}
inline Vector2 Vector2::MirrorByVector(const Vector2& Normal) const
{
	return *this - Normal * (2 * (*this | Normal));
}

Vector3 CrossProduct(Vector3& v1, Vector3& v2)
{
	return v1 ^ v2;
}


inline double Vector3::operator[](const int Index) const
{
	switch (Index) {
	case 0: return X;
	case 1: return Y;
	case 2: return Z;
	default: throw std::out_of_range("Index for vector out of array (must be in range 0-2)");
	}

}
inline double Vector2::operator[](const int Index) const
{
	switch (Index) {
	case 0: return X;
	case 1: return Y;
	default: throw std::out_of_range("Index for vector out of array (must be in range 0-1)");
	}
}

inline Vector3 Vector3::operator-() const
{
	return { -X, -Y, -Z };
}
inline Vector2 Vector2::operator-() const
{
	return { -X, -Y };
}


inline Vector3 Vector3::operator+(const Vector3& Other) const
{
	return { X + Other.X, Y + Other.Y, Z + Other.Z };
}
inline Vector2 Vector2::operator+(const Vector2& Other) const
{
	return { X + Other.X, Y + Other.Y };
}

inline Vector3 Vector3::operator-(const Vector3& Other) const
{
	return { X - Other.X, Y - Other.Y, Z - Other.Z };
}
inline Vector2 Vector2::operator-(const Vector2& Other) const
{
	return { X - Other.X, Y - Other.Y };
}

inline Vector3 Vector3::operator*(const Vector3& Other) const
{
	return { X * Other.X, Y * Other.Y, Z * Other.Z };
}
inline Vector2 Vector2::operator*(const Vector2& Other) const
{
	return { X * Other.X, Y * Other.Y };
}

inline Vector3 Vector3::operator/(const Vector3& Other) const
{
	return { X / Other.X, Y / Other.Y, Z / Other.Z };
}
inline Vector2 Vector2::operator/(const Vector2& Other) const
{
	return { X / Other.X, Y / Other.Y };
}

inline Vector3 Vector3::operator^(const Vector3& Other) const
{
	return
	{
		Y * Other.Z - Z * Other.Y,
		Z * Other.X - X * Other.Z,
		X * Other.Y - Y * Other.X
	};
}

inline double Vector3::operator|(const Vector3& Other) const
{
	return X * Other.X + Y * Other.Y + Z * Other.Z;
}
inline double Vector2::operator|(const Vector2& Other) const
{
	return X * Other.X + Y * Other.Y;
}

inline Vector3 Vector3::operator*(const double Scale) const
{
	return { X * Scale, Y * Scale, Z * Scale };
}
inline Vector2 Vector2::operator*(const double Scale) const
{
	return { X * Scale, Y * Scale };
}

inline Vector3 Vector3::operator/(const double Scale) const
{
	return { X / Scale, Y / Scale, Z / Scale };
}
inline Vector2 Vector2::operator/(const double Scale) const
{
	return { X / Scale, Y / Scale };
}

inline bool Vector3::operator==(const Vector3& Other) const
{
	return this->Equals(Other, SMALL_NUMBER);
}
inline bool Vector2::operator==(const Vector2& Other) const
{
	return this->Equals(Other, SMALL_NUMBER);
}

inline bool Vector3::operator!=(const Vector3& Other) const
{
	return !(*this == Other);
}
inline bool Vector2::operator!=(const Vector2& Other) const
{
	return !(*this == Other);
}

inline bool Vector3::operator<(const Vector3& Other) const
{
	return Length() < Other.Length();
}
inline bool Vector2::operator<(const Vector2& Other) const
{
	return Length() < Other.Length();
}

inline bool Vector3::operator>(const Vector3& Other) const
{
	return Length() > Other.Length();
}
inline bool Vector2::operator>(const Vector2& Other) const
{
	return Length() < Other.Length();
}

inline bool Vector3::operator<=(const Vector3& Other) const
{
	return Length() <= Other.Length();
}
inline bool Vector2::operator<=(const Vector2& Other) const
{
	return Length() <= Other.Length();
}

inline bool Vector3::operator>=(const Vector3& Other) const
{
	return Length() >= Other.Length();
}
inline bool Vector2::operator>=(const Vector2& Other) const
{
	return Length() >= Other.Length();
}

inline Vector3 operator*(const double Scale, const Vector3& V)
{
	return V * Scale;
}
inline Vector2 operator*(const double Scale, const Vector2& V)
{
	return V * Scale;
}
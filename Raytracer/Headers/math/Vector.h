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

	double GetMax() const;
	double GetMin() const;

	Vector2 MaxAxis() const;
	Vector2 MinAxis() const;

	/*Get a copy with sign components*/
	Vector2 Sign() const;

	static Vector2 Abs(const Vector2& V);

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

	Vector2& operator+=(const Vector2& Other);
	Vector2& operator*=(const Vector2& Other);
	Vector2& operator/=(const Vector2& Other);
	Vector2& operator-=(const Vector2& Other);
	Vector2& operator*=(const double& Scale);
	Vector2& operator/=(const double& Scale);


	static const Vector2 I;
	static const Vector2 J;

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
	
	/*
	  mirror vector by an input normal
	  input normal is automatically Normalized
	*/
	Vector3 MirrorByVector(const Vector3& Normal) const;

	double GetMax() const;
	double GetMin() const;

	Vector3 MaxAxis() const;
	Vector3 MinAxis() const;

	bool NearlyZero(const double Tolerance) const;

	/*Get a copy with sign components*/
	Vector3 Sign() const;

	static Vector3 Abs(const Vector3& V);


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

	Vector3& operator+=(const Vector3& Other);
	Vector3& operator*=(const Vector3& Other);
	Vector3& operator/=(const Vector3& Other);
	Vector3& operator-=(const Vector3& Other);
	Vector3& operator*=(const double& Scale);
	Vector3& operator/=(const double& Scale);

	static const Vector3 I;
	static const Vector3 J;
	static const Vector3 K;
};

struct Quaternion
{
	double X;
	double Y;
	double Z;
	double W;

public:
	Quaternion() : X(0.0), Y(0.0), Z(0.0), W(0.0) {}
	Quaternion(double InX, double InY, double InZ, double InW) : X(InX), Y(InY), Z(InZ), W(InW) {}

	Quaternion operator+(const Quaternion& Other) const;
	Quaternion operator-(const Quaternion& Other) const;
	Quaternion operator*(const Quaternion& Other) const;
	Quaternion operator/(const Quaternion& Other) const;

	Vector3 RotateVector(const Vector3& V) const;
	Vector3 UnrotateVector(const Vector3& V) const;
};

inline const Vector3 Vector3::I = { 1.0, 0.0, 0.0 };
inline const Vector3 Vector3::J = { 0.0, 1.0, 0.0 };
inline const Vector3 Vector3::K = { 0.0, 0.0, 1.0 };

inline const Vector2 Vector2::I = { 1.0, 0.0 };
inline const Vector2 Vector2::J = { 0.0, 1.0 };


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
	double Scale = 0;
	if (!(std::abs(L) < SMALL_NUMBER)) Scale = 1.0 / L;
		
	return { X * Scale, Y * Scale, Z * Scale };
}
inline Vector2 Vector2::Normalized() const
{
	const double L = Length();
	double Scale = 0;
	if (!(std::abs(L) < SMALL_NUMBER)) Scale = 1.0 / L;

	return { X * Scale, Y * Scale };
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

inline double Vector3::GetMax() const
{
	const double Max = X > Y ? X : Y;
	return Max > Z ? Max : Z;
}
inline double Vector2::GetMax() const
{
	return X > Y ? X : Y;
}

inline double Vector3::GetMin() const
{
	const double Min = X < Y ? X : Y;
	return Min < Z ? Min : Z;
}
inline double Vector2::GetMin() const
{
	return X < Y ? X : Y;
}

inline Vector3 Vector3::MaxAxis() const
{
	int MaxInd = 0;
	const Vector3 AbsVector = Vector3::Abs(*this);
	if (AbsVector.Y > AbsVector.X) MaxInd = 1;
	if (AbsVector.Z > AbsVector[MaxInd]) MaxInd = 2;
	switch(MaxInd)
	{
	case 0: return Vector3::I * this->Sign();
	case 1: return Vector3::J * this->Sign();
	case 2: return Vector3::K * this->Sign();
	default: return { 0.0, 0.0, 0.0 }; //Will never be executed, though
	}
}
inline Vector2 Vector2::MaxAxis() const
{
	const Vector2 AbsVector = Vector2::Abs(*this);
	if(AbsVector.X > AbsVector.Y)
	{
		return Vector2::I;
	}
	else
	{
		return Vector2::J;
	}
}

inline Vector3 Vector3::MinAxis() const
{
	int MinInd = 0;
	const Vector3 AbsVector = Vector3::Abs(*this);
	if (AbsVector.Y < AbsVector.X) MinInd = 1;
	if (AbsVector.Z < AbsVector[MinInd]) MinInd = 2;
	switch (MinInd)
	{
	case 0: return X < 0 ? Vector3::I : -Vector3::I;
	case 1: return Y < 0 ? Vector3::J : -Vector3::J;
	case 2: return Z < 0 ? Vector3::K : -Vector3::K;
	default: return { 0.0, 0.0, 0.0 }; //Will never be executed, though
	}
}

inline bool Vector3::NearlyZero(const double Tolerance = 1e-6) const
{
	return std::abs(X) < Tolerance && std::abs(Y) < Tolerance && std::abs(Z) < Tolerance;
}

inline Vector2 Vector2::MinAxis() const
{
	const Vector2 AbsVector = Vector2::Abs(*this);
	if (AbsVector.X < AbsVector.Y)
	{
		return Vector2::I;
	}
	else
	{
		return Vector2::J;
	}
}

inline Vector3 Vector3::Sign() const
{
	return {
		static_cast<double>(0 < X) - static_cast<double>(X < 0),
		static_cast<double>(0 < Y) - static_cast<double>(Y < 0),
		static_cast<double>(0 < Z) - static_cast<double>(Z < 0)
	};
}
inline Vector2 Vector2::Sign() const
{
	return {
		static_cast<double>(0 < X) - static_cast<double>(X < 0),
		static_cast<double>(0 < Y) - static_cast<double>(Y < 0)
	};
}

inline Vector3 Vector3::Abs(const Vector3& V)
{
	return {
		std::abs(V.X),
		std::abs(V.Y),
		std::abs(V.Z)
	};
}
inline Vector2 Vector2::Abs(const Vector2& V)
{
	return {
		std::abs(V.X),
		std::abs(V.Y)
	};
}

inline Vector3 CrossProduct(Vector3& v1, Vector3& v2)
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



inline Vector3& Vector3::operator+=(const Vector3& Other)
{
	return (*this = *this + Other);
}
inline Vector3& Vector3::operator*=(const Vector3& Other)
{
	return (*this = *this * Other);
}
inline Vector3& Vector3::operator/=(const Vector3& Other)
{
	return (*this = *this / Other);
}
inline Vector3& Vector3::operator-=(const Vector3& Other)
{
	return (*this = *this - Other);
}

inline Vector3& Vector3::operator*=(const double& Scale)
{
	return (*this = *this * Scale);
}

inline Vector3& Vector3::operator/=(const double& Scale)
{
	return (*this = *this / Scale);
}





inline Vector2& Vector2::operator+=(const Vector2& Other)
{
	return (*this = *this + Other);
}
inline Vector2& Vector2::operator*=(const Vector2& Other)
{
	return (*this = *this * Other);
}
inline Vector2& Vector2::operator/=(const Vector2& Other)
{
	return (*this = *this / Other);
}
inline Vector2& Vector2::operator-=(const Vector2& Other)
{
	return (*this = *this - Other);
}

inline Vector2& Vector2::operator*=(const double& Scale)
{
	return (*this = *this * Scale);
}

inline Vector2& Vector2::operator/=(const double& Scale)
{
	return (*this = *this / Scale);
}



inline Vector3 operator*(const double Scale, const Vector3& V)
{
	return V * Scale;
}
inline Vector2 operator*(const double Scale, const Vector2& V)
{
	return V * Scale;
}









inline Vector3 Quaternion::RotateVector(const Vector3& V) const
{
	const Vector3 Q(X, Y, Z);
	const Vector3 T = 2.0 * (Q ^ V);
	const Vector3 Result = V + (W * T) + (Q ^ T);
	return Result;
}

inline Vector3 Quaternion::UnrotateVector(const Vector3& V) const
{
	const Vector3 Q(-X, -Y, -Z);
	const Vector3 T = 2.0 * (Q ^ V);
	const Vector3 Result = V + (W * T) + (Q ^ T);
	return Result;
}

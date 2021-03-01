#pragma once
#include "Vector.h"

struct Matrix4x4
{
public:
	Matrix4x4() 
	{
		SetIdentity();
	};

	Matrix4x4(
		const Quaternion& R1,
		const Quaternion& R2,
		const Quaternion& R3,
		const Quaternion& R4
		);

	static const Matrix4x4 Identity;

	void SetIdentity();
	void SetTranslation(const Vector3& Translation);
	void SetScale(const Vector3& Scale);
	void SetRotationX(const double Rotation);
	void SetRotationY(const double Rotation);
	void SetRotationZ(const double Rotation);
	Matrix4x4 T() const;

	Matrix4x4 operator*(const Matrix4x4 Other) const;
	double* operator[](const uint8_t Index);

private:
	double Matrix[4][4];
};

inline const Matrix4x4 Matrix4x4::Identity = Matrix4x4(
	Quaternion(1.0, 0.0, 0.0, 0.0),
	Quaternion(0.0, 1.0, 0.0, 0.0),
	Quaternion(0.0, 0.0, 1.0, 0.0),
	Quaternion(0.0, 0.0, 0.0, 1.0));

inline Matrix4x4::Matrix4x4(
	const Quaternion& R1,
	const Quaternion& R2,
	const Quaternion& R3,
	const Quaternion& R4
)
{
	Matrix[0][0] = R1.X; Matrix[0][1] = R1.Y; Matrix[0][2] = R1.Z; Matrix[0][3] = R1.W;
	Matrix[1][0] = R2.X; Matrix[1][1] = R2.Y; Matrix[1][2] = R2.Z; Matrix[1][3] = R2.W;
	Matrix[2][0] = R3.X; Matrix[2][1] = R3.Y; Matrix[2][2] = R3.Z; Matrix[2][3] = R3.W;
	Matrix[3][0] = R4.X; Matrix[3][1] = R4.Y; Matrix[3][2] = R4.Z; Matrix[3][3] = R4.W;
}

inline void Matrix4x4::SetIdentity()
{
	*this = Identity;
}

inline void Matrix4x4::SetTranslation(const Vector3& Translation)
{
	SetIdentity();
	Matrix[3][0] = Translation.X;
	Matrix[3][1] = Translation.Y;
	Matrix[3][2] = Translation.Z;
}

inline void Matrix4x4::SetScale(const Vector3& Scale)
{
	SetIdentity();
	Matrix[0][0] = Scale.X;
	Matrix[1][1] = Scale.Y;
	Matrix[2][2] = Scale.Z;
}

inline void Matrix4x4::SetRotationX(const double Rotation)
{
	SetIdentity();
	double RadRotation = Rotation / PI * 180;
	Matrix[1][1] = std::cos(RadRotation);
	Matrix[1][2] = std::sin(RadRotation);
	Matrix[2][1] = -std::sin(RadRotation);
	Matrix[2][2] = std::cos(RadRotation);
}

inline void Matrix4x4::SetRotationY(const double Rotation)
{
	SetIdentity();
	double RadRotation = Rotation / PI * 180;
	Matrix[0][0] = std::cos(RadRotation);
	Matrix[0][2] = -std::sin(RadRotation);
	Matrix[2][0] = std::sin(RadRotation);
	Matrix[2][2] = std::cos(RadRotation);
}

inline void Matrix4x4::SetRotationZ(const double Rotation)
{
	SetIdentity();
	double RadRotation = Rotation / PI * 180;
	Matrix[0][0] = std::cos(RadRotation);
	Matrix[0][1] = std::sin(RadRotation);
	Matrix[1][0] = -std::sin(RadRotation);
	Matrix[1][1] = std::cos(RadRotation);
}

inline Matrix4x4 Matrix4x4::T() const
{
	Matrix4x4 NewMatrix;
	for(uint8_t i = 0; i < 4; i++)
		for (uint8_t j = 0; j < 4; j++)
			NewMatrix.Matrix[i][j] = Matrix[j][i];
	return NewMatrix;
}

inline Matrix4x4 Matrix4x4::operator*(const Matrix4x4 Other) const
{
	Matrix4x4 NewMatrix;
	for (uint8_t i = 0; i < 4; i++)
		for (uint8_t j = 0; j < 4; j++)
		{
			NewMatrix.Matrix[i][j] = Matrix[i][0] * Other.Matrix[j][0] + \
									 Matrix[i][1] * Other.Matrix[j][1] + \
									 Matrix[i][2] * Other.Matrix[j][2] + \
									 Matrix[i][3] * Other.Matrix[j][3];
		}
	return NewMatrix;
}

inline double* Matrix4x4::operator[](const uint8_t Index)
{
	return Matrix[Index];
}
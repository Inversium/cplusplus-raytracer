#pragma once
#include "math/Vector.h"
#include "math/Matrix.h"


struct RTransform
{
	RTransform() : Position(), Rotation(), Scale(1.0) {};
	RTransform(const Vector3& InPosition, const Vector3& InRotation, const Vector3& InScale);

private:
	Vector3 Position;
	Quaternion Rotation;
	Vector3 Scale;

public:
	Vector3 GetPosition() const;
	Vector3 GetRotation() const;
	Vector3 GetScale() const;

	void SetPosition(const Vector3& NewPosition);
	void SetPosition(const double NewX, const double NewY, const double NewZ);
	void SetRotation(const Vector3& NewRotation);
	void SetRotation(const double NewXRot, const double NewYRot, const double NewZRot);
	void SetScale(const Vector3& NewScale);
	void SetScale(const double NewXScale, const double NewYScale, const double NewZScale);

	void AddPosition(const Vector3& DeltaPosition);

	Matrix4x4 ToMatrix() const;

	static Quaternion EulerToQuaternion(const Vector3& Rotation);
	static Vector3 EulerFromQuaternion(const Quaternion& Quat);

	Vector3 TransformVectorNoScale(const Vector3& V) const;
	Vector3 TransformVector(const Vector3& V) const;
	Vector3 TransformPosition(const Vector3& V) const;
	Vector3 InverseTransformVectorNoScale(const Vector3& V) const;
	Vector3 InverseTransformVector(const Vector3& V) const;
	Vector3 InverseTransformPosition(const Vector3& V) const;

private:
	


};

inline RTransform::RTransform(const Vector3& InPosition, const Vector3& InRotation, const Vector3& InScale)
{
	Position = InPosition;
	Rotation = EulerToQuaternion(InRotation);
	Scale = InScale;
}

inline Vector3 RTransform::GetPosition() const
{
	return Position;
}

inline Vector3 RTransform::GetRotation() const
{
	return EulerFromQuaternion(Rotation);
}

inline Vector3 RTransform::GetScale() const
{
	return Scale;
}

inline void RTransform::SetPosition(const Vector3& NewPosition)
{
	Position = NewPosition;
}

inline void RTransform::SetPosition(const double NewX, const double NewY, const double NewZ)
{
	SetPosition(Vector3(NewX, NewY, NewZ));
}

inline void RTransform::SetRotation(const Vector3& NewRotation)
{
	Rotation = EulerToQuaternion(NewRotation);
}

inline void RTransform::SetRotation(const double NewXRot, const double NewYRot, const double NewZRot)
{
	SetRotation(Vector3(NewXRot, NewYRot, NewZRot));
}

inline void RTransform::SetScale(const Vector3& NewScale)
{
	Scale = NewScale;
}

inline void RTransform::SetScale(const double NewXScale, const double NewYScale, const double NewZScale)
{
	SetScale(Vector3(NewXScale, NewYScale, NewZScale));
}

inline void RTransform::AddPosition(const Vector3& DeltaPosition)
{
	Position = Position + DeltaPosition;
}

inline Matrix4x4 RTransform::ToMatrix() const
{
	Matrix4x4 OutMatrix;

	OutMatrix[3][0] = Position.X;
	OutMatrix[3][1] = Position.X;
	OutMatrix[3][2] = Position.X;

	const double X2 = Rotation.X + Rotation.X;
	const double Y2 = Rotation.Y + Rotation.Y;
	const double Z2 = Rotation.Z + Rotation.Z;
	{
		const double xx2 = Rotation.X * X2;
		const double yy2 = Rotation.Y * Y2;
		const double zz2 = Rotation.Z * Z2;

		OutMatrix[0][0] = (1.0f - (yy2 + zz2)) * Scale.X;
		OutMatrix[1][1] = (1.0f - (xx2 + zz2)) * Scale.Y;
		OutMatrix[2][2] = (1.0f - (xx2 + yy2)) * Scale.Z;
	}
	{
		const double yz2 = Rotation.Y * Z2;
		const double wx2 = Rotation.W * X2;

		OutMatrix[2][1] = (yz2 - wx2) * Scale.Z;
		OutMatrix[1][2] = (yz2 + wx2) * Scale.Y;
	}
	{
		const double xy2 = Rotation.X * Y2;
		const double wz2 = Rotation.W * Z2;

		OutMatrix[1][0] = (xy2 - wz2) * Scale.Y;
		OutMatrix[0][1] = (xy2 + wz2) * Scale.X;
	}
	{
		const double xz2 = Rotation.X * Z2;
		const double wy2 = Rotation.W * Y2;

		OutMatrix[2][0] = (xz2 + wy2) * Scale.Z;
		OutMatrix[0][2] = (xz2 - wy2) * Scale.X;
	}

	OutMatrix[0][3] = 0.0f;
	OutMatrix[1][3] = 0.0f;
	OutMatrix[2][3] = 0.0f;
	OutMatrix[3][3] = 1.0f;

	return OutMatrix;
}

inline Quaternion RTransform::EulerToQuaternion(const Vector3& Rotation)
{
	/* See https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles */

	/* Convert degrees to radians */
	const double Roll = Rotation.X * PI / 180.0;
	const double Pitch = Rotation.Y * PI / 180.0;
	const double Yaw = Rotation.Z * PI / 180.0;

	const double cy = std::cos(Yaw * 0.5);
	const double sy = std::sin(Yaw * 0.5);
	const double cp = std::cos(Pitch * 0.5);
	const double sp = std::sin(Pitch * 0.5);
	const double cr = std::cos(Roll * 0.5);
	const double sr = std::sin(Roll * 0.5);

	Quaternion Quat;
	Quat.W = cy * cp * cr + sy * sp * sr;
	Quat.X = cy * cp * sr - sy * sp * cr;
	Quat.Y = sy * cp * sr + cy * sp * cr;
	Quat.Z = sy * cp * cr - cy * sp * sr;

	return Quat;
}

inline Vector3 RTransform::EulerFromQuaternion(const Quaternion& Quat)
{ 
	/* See https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles */

	Vector3 Rotation;

	// roll (x-axis rotation)
	double sinr_cosp = 2 * (Quat.W * Quat.X + Quat.Y * Quat.Z);
	double cosr_cosp = 1 - 2 * (Quat.X * Quat.X + Quat.Y * Quat.Y);
	Rotation.X = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = 2 * (Quat.W * Quat.Y - Quat.Z * Quat.X);
	if (std::abs(sinp) >= 1)
		Rotation.Y = std::copysign(PI / 2, sinp); // use 90 degrees if out of range
	else
		Rotation.Y = std::asin(sinp);

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (Quat.W * Quat.Z + Quat.X * Quat.Y);
	double cosy_cosp = 1 - 2 * (Quat.Y * Quat.Y + Quat.Z * Quat.Z);
	Rotation.Z = std::atan2(siny_cosp, cosy_cosp);

	return Rotation;
}

inline Vector3 RTransform::TransformVectorNoScale(const Vector3& V) const
{
	return Rotation.RotateVector(V);
}

inline Vector3 RTransform::TransformVector(const Vector3& V) const
{
	return Rotation.RotateVector(V * Scale);
}

inline Vector3 RTransform::TransformPosition(const Vector3& V) const
{
	return Rotation.RotateVector(V * Scale) + Position;
}

inline Vector3 RTransform::InverseTransformVectorNoScale(const Vector3& V) const
{
	return Rotation.UnrotateVector(V);
}

inline Vector3 RTransform::InverseTransformVector(const Vector3& V) const
{
	return Rotation.UnrotateVector(V) / Scale;
}

inline Vector3 RTransform::InverseTransformPosition(const Vector3& V) const
{
	return Rotation.UnrotateVector(V - Position) / Scale;
}


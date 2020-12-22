#pragma once
#include <cmath>
#include <string>
#include <stdexcept>

#define PI 3.1415926535
#define SMALL_NUMBER 1e-7



struct vec2 
{
	double x, y;

	vec2(double x = 0, double y = 0) : x(x), y(y) {}

	vec2(double value) : x(value), y(value) {}

	//returns length of this vector
	double getLength() const;

	//returns a normalized copy of an input vector
	vec2 normalized() const;

	//returns a human readable string for debugging
	std::string toString() const;

	/*
	  if vector length greater the max param, set length of this vector to the max
	  if vector length lesser the min param, set length of this vector to the min
	  otherwise do nothing
	*/
	vec2 Clamp(double min, double max);

	//returns true if all components of two vectors are equal within the given tolerance
	bool Equals(const vec2 &other, double tolerance = 1e-6) const;

	//return true if vector length == 1
	bool isNormalized() const;
	
	/*
	  mirror vector by an input normal
	  input normal is automatically normalized
	*/
	vec2 MirrorByVector(vec2 &normal);

	double operator[](const int index) const;
	vec2 operator-() const;

	vec2 operator+(const vec2 &other) const;
	vec2 operator-(const vec2 &other) const;
	vec2 operator*(const vec2 &other) const;
	double operator|(const vec2 &other) const;

	vec2 operator*(const double scale) const;
	vec2 operator/(const double scale) const;

	bool operator==(const vec2 &other) const;
	bool operator!=(const vec2 &other) const;

};

struct vec3
{

	double x, y, z;

	vec3() { x = 0.0; y = 0.0; z = 0.0; }

	vec3(double x, double y, double z) : x(x), y(y), z(z) {}

	vec3(double value) : x(value), y(value), z(value) {}

    vec3(vec2 &v, double z) : x(v.x), y(v.y), z(z) {}

	
	//returns length of this vector
	double getLength() const;

	//returns a normalized copy of an input vector
	vec3 normalized() const;

	//returns a human readable string for debugging
	std::string toString() const;

	/*
	  if vector length greater the max param, set length of this vector to the max
	  if vector length lesser the min param, set length of this vector to the min
	  otherwise do nothing
	*/ 
	vec3 Clamp(double min, double max);
	
	//returns true if all components of two vectors are equal within the given tolerance
	bool Equals(const vec3 &other, double tolerance = 1e-6) const;
	
	//return true if vector length == 1
	bool isNormalized() const;

	/*
	  mirror vector by an input normal
	  input normal is automatically normalized
	*/
	vec3 MirrorByVector(vec3 &normal);



	double operator[](const int index) const;
	vec3 operator-() const;

	vec3 operator/(const vec3& other) const;
	vec3 operator+(const vec3 &other) const;
	vec3 operator-(const vec3 &other) const;
	vec3 operator*(const vec3 &other) const;
	double operator|(const vec3 &other) const;
	vec3 operator^(const vec3 &other) const;

	vec3 operator*(const double scale) const;
	vec3 operator/(const double scale) const;

	bool operator==(const vec3 &other) const;
	bool operator!=(const vec3 &other) const;
	bool operator<(const vec3& other) const;
	bool operator>(const vec3& other) const;
};


inline double vec3::getLength() const 
{
	return sqrt(x * x + y * y + z * z);
}
inline double vec2::getLength() const
{
	return sqrt(x * x + y * y);
}

inline vec3 vec3::normalized() const
{
	double l = getLength();
	return vec3(x / l, y / l, z / l);
}
inline vec2 vec2::normalized() const
{
	double l = getLength();
	return vec2(x / l, y / l);
}

inline std::string vec3::toString() const
{
	std::string out = "";
	out += "X = " + std::to_string(x);
	out += " Y = " + std::to_string(y);
	out += " Z = " + std::to_string(z);
	return out;
}
inline std::string vec2::toString() const
{
	std::string out = "";
	out += "X = " + std::to_string(x);
	out += " Y = " + std::to_string(y);
	return out;
}

inline vec3 vec3::Clamp(double min, double max)
{
	if (getLength() > max)
	{
		*this = this->normalized() * max;
		return *this;
	}
	else if (getLength() < min)
	{
		*this = this->normalized() * min;
		return *this;
	}
	return *this;
}
inline vec2 vec2::Clamp(double min, double max)
{
	if (getLength() > max)
	{
		*this = this->normalized() * max;
		return *this;
	}
	else if (getLength() < min)
	{
		*this = this->normalized() * min;
		return *this;
	}
	return *this;
}

inline bool vec3::Equals(const vec3 &other, double tolerance) const
{
    return std::abs(other.x - x) <= tolerance && std::abs(other.y - y) <= tolerance && std::abs(other.y - y) <= tolerance;
}
inline bool vec2::Equals(const vec2 &other, double tolerance) const
{
    return std::abs(other.x - x) <= tolerance && std::abs(other.y - y) <= tolerance;
}

inline bool vec3::isNormalized() const
{
    return std::abs(1 - getLength()) < SMALL_NUMBER;
}
inline bool vec2::isNormalized() const
{
    return std::abs(1 - getLength()) < SMALL_NUMBER;
} 

inline vec3 vec3::MirrorByVector(vec3 &normal)
{
	return *this - normal * (2 * (*this | normal));
}
inline vec2 vec2::MirrorByVector(vec2 &normal)
{
	return *this - normal * (2 * (*this | normal));
}


inline double vec3::operator[](const int index) const
{
    switch (index) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: throw std::out_of_range("Index for vector out of array (must be in range 0-2)");
    }

}
inline double vec2::operator[](const int index) const
{
    switch (index) {
    case 0: return x;
    case 1: return y;
    default: throw std::out_of_range("Index for vector out of array (must be in range 0-1)");
    }
}

inline vec3 vec3::operator-() const
{
	return vec3(-x, -y, -z);
}
inline vec2 vec2::operator-() const
{
	return vec2(-x, -y);
}


inline vec3 vec3::operator/(const vec3& other) const
{
	return vec3(x / other.x, y / other.y, z / other.z);
}

inline vec3 vec3::operator+(const vec3 &other) const
{
	return vec3(x + other.x, y + other.y, z + other.z);
}
inline vec2 vec2::operator+(const vec2 &other) const
{
	return vec2(x + other.x, y + other.y);
}

inline vec3 vec3::operator-(const vec3 &other) const
{
	return vec3(x - other.x, y - other.y, z - other.z);
}
inline vec2 vec2::operator-(const vec2 &other) const
{
	return vec2(x - other.x, y - other.y);
}

inline vec3 vec3::operator*(const vec3 &other) const
{
	return vec3(x * other.x, y * other.y, z * other.z);
}
inline vec2 vec2::operator*(const vec2 &other) const
{
	return vec2(x * other.x, y * other.y);
}

inline vec3 vec3::operator^(const vec3 &other) const
{
	return vec3
	(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

inline double vec3::operator|(const vec3 &other) const
{
	return x * other.x + y * other.y + z * other.z;
}
inline double vec2::operator|(const vec2 &other) const
{
	return x * other.x + y * other.y;
}

inline vec3 vec3::operator*(const double scale) const
{
	return vec3(x * scale, y * scale, z * scale);
}
inline vec2 vec2::operator*(const double scale) const
{
	return vec2(x * scale, y * scale);
}

inline vec3 vec3::operator/(const double scale) const
{
	return vec3(x / scale, y / scale, z / scale);
}
inline vec2 vec2::operator/(const double scale) const
{
	return vec2(x / scale, y / scale);
}

inline bool vec3::operator==(const vec3 &other) const
{
	return x == other.x && y == other.y && z == other.z;
}
inline bool vec2::operator==(const vec2 &other) const
{
	return x == other.x && y == other.y;
}

inline bool vec3::operator!=(const vec3 &other) const
{
	return !(*this == other);
}
inline bool vec3::operator<(const vec3& other) const
{
	return this->getLength() < other.getLength();
}
inline bool vec3::operator>(const vec3& other) const
{
	return this->getLength() > other.getLength();
}
inline bool vec2::operator!=(const vec2 &other) const
{
	return !(*this == other);
}



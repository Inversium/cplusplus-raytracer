#pragma once
#include "mvector.h"

namespace MMath 
{
	//dot product of two vectors
	double dot(const vec3 &v1, const vec3 &v2);
	double dot(const vec2 &v1, const vec2 &v2);

	//in degrees
	double angle(const vec3 &v1, const vec3 &v2);
	double angle(const vec2 &v1, const vec2 &v2);

	//cross product of two vectors
	vec3 CrossProduct(const vec3 &v1, const vec3 &v2);
}
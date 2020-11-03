#include "../Headers/mvector.h"
#include "../Headers/mmath.h"



double MMath::angle(const vec3 &v1, const vec3 &v2)
{
	vec3 a = v1.normalized();
	vec3 b = v2.normalized();
	double value = dot(a, b) / (a.getLength() * a.getLength());
	if (value < -1) value = -1;
	if (value > 1) value = 1;
	return acos(value) * 180 / PI;
}
double MMath::angle(const vec2 &v1, const vec2 &v2)
{
	vec2 a = v1.normalized();
	vec2 b = v2.normalized();
	double value = dot(a, b) / (a.getLength() * a.getLength());
	if (value < -1) value = -1;
	if (value > 1) value = 1;
	return acos(value) * 180 / PI;
}

double MMath::dot(const vec3 &v1, const vec3 &v2)
{
	return v1 | v2;
}
double MMath::dot(const vec2 &v1, const vec2 &v2)
{
	return v1 | v2;
}

vec3 MMath::CrossProduct(const vec3 &v1, const vec3 &v2)
{
	return v1 ^ v2;
}
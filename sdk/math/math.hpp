#pragma once
#include "pch.hpp"
#define M_PI		3.14159265358979323846
#define M_PI_R		0.01745329251
#define M_PI_F		((float)(M_PI))

inline auto d2r(float Degrees) { return Degrees * (M_PI_F / 180.0f); }
inline auto r2d(float Radians) { return Radians * (180.0f / M_PI_F); }

struct vec3_t {
	int x, y, z;
};

inline vec3_t operator+(const vec3_t& a, const vec3_t& b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline vec3_t operator-(const vec3_t& a, const vec3_t& b) {
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline vec3_t operator*(const vec3_t& a, const vec3_t& b) {
	return { a.x * b.x, a.y * b.y, a.z * b.z };
}

inline vec3_t operator/(const vec3_t& a, const vec3_t& b) {
	return { a.x / b.x, a.y / b.y, a.z / b.z };
}

struct vec2_t {
	int x, y;
};

inline vec2_t operator+(const vec2_t& a, const vec2_t& b) {
	return { a.x + b.x, a.y + b.y };
}

inline vec2_t operator-(const vec2_t& a, const vec2_t& b) {
	return { a.x - b.x, a.y - b.y };
}

inline vec2_t operator*(const vec2_t& a, const vec2_t& b) {
	return { a.x * b.x, a.y * b.y };
}

inline vec2_t operator/(const vec2_t& a, const vec2_t& b) {
	return { a.x / b.x, a.y / b.y };
}

inline float NormalizeYaw(float Yaw) {
	if (!std::isfinite(Yaw))
		return 0.0f;

	if (Yaw >= -180.f && Yaw <= 180.f)
		return Yaw;

	const float rot = std::round(std::abs(Yaw / 360.f));

	Yaw = (Yaw < 0.f) ? Yaw + (360.f * rot) : Yaw - (360.f * rot);
	return Yaw;
}

inline void VectorAngles(const vec3_t& forward, vec3_t& angles)
{
	float tmp, yaw, pitch;

	if (forward.y == 0 && forward.x == 0)
	{
		yaw = 0;
		if (forward.z > 0) pitch = 270;
		else pitch = 90;
	}
	else
	{
		yaw = (atan2(forward.y, forward.x) * 180 / M_PI);
		if (yaw < 0) yaw += 360;
		tmp = sqrt(forward.x * forward.x + forward.y * forward.y);
		pitch = (atan2(-forward.z, tmp) * 180 / M_PI);
		if (pitch < 0) pitch += 360;
	}

	angles.x = pitch;
	angles.y = yaw;
	angles.z = 0;
}

inline void SinCos(float r, float* s, float* c) {
	*s = sin(r);
	*c = cos(r);
}

inline void AngleVectors(const vec3_t& angles, vec3_t* forward) {
	float sp, sy, cp, cy;
	SinCos(d2r(angles.x), &sp, &cp);
	SinCos(d2r(angles.y), &sy, &cy);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}
/*
inline void AngleVectors(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up)
{
	float sr, sp, sy, cr, cp, cy;

	SinCos(d2r(angles[YAW]), &sy, &cy);
	SinCos(d2r(angles[PITCH]), &sp, &cp);
	SinCos(d2r(angles[ROLL]), &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}
*/
/*
bool WorldToScreen(const vec3_t& world,vec2_t& screen,const matrix4x4_t& worldToScreen,float screenWidth,float screenHeight)
{
	const float clipX =
		worldToScreen[0][0] * world.x +
		worldToScreen[0][1] * world.y +
		worldToScreen[0][2] * world.z +
		worldToScreen[0][3];

	const float clipY =
		worldToScreen[1][0] * world.x +
		worldToScreen[1][1] * world.y +
		worldToScreen[1][2] * world.z +
		worldToScreen[1][3];

	const float clipW =
		worldToScreen[3][0] * world.x +
		worldToScreen[3][1] * world.y +
		worldToScreen[3][2] * world.z +
		worldToScreen[3][3];

	if (clipW <= 0.001f) return false;
	const float invW = 1.0f / clipW;
	const float ndcX = clipX * invW;
	const float ndcY = clipY * invW;

	screen.x = (ndcX + 1.0f) * 0.5f * screenWidth;
	screen.y = (1.0f - ndcY) * 0.5f * screenHeight;

	return true;
}
*/

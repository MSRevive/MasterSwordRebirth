//========= Copyright (c) 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

//#ifndef PITCH
// MOVEMENT INFO
// up / down
enum {
	PITCH = 0,
	// left / right
	YAW = 1,
	// fall over
	ROLL = 2
};
//#endif

#undef PLANE_ANYZ

enum planes_e {
	PLANE_X = 0,
	PLANE_Y = 1,
	PLANE_Z = 2,
	PLANE_ANYZ = 5
};

template <typename Type>
inline Type FDotProduct(const Type a, const Type b) {
	return fabs(a[0] * b[0]) + fabs(a[1] * b[1]) + fabs(a[2] * b[2]);
}

void AngleMatrix(const float *angles, float (*matrix)[4]);
bool VectorCompare (const float *v1, const float *v2);
void CrossProduct(const float *v1, const float *v2, float *cross);
void VectorTransform(const float *in1, float in2[3][4], float *out);
void ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);
void MatrixCopy(float in[3][4], float out[3][4]);
void QuaternionMatrix(vec4_t quaternion, float (*matrix)[4]);
void QuaternionSlerp(vec4_t p, vec4_t q, float t, vec4_t qt);
void AngleQuaternion(float *angles, vec4_t quaternion);
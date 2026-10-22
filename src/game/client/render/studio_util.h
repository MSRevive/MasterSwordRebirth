//========= Copyright (c) 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "mathlib.h"

#define FDotProduct(a, b) (fabs((a[0]) * (b[0])) + fabs((a[1]) * (b[1])) + fabs((a[2]) * (b[2])))

void AngleMatrix(const float *angles, float (*matrix)[4]);
void CrossProduct(const float *v1, const float *v2, float *cross);
void VectorTransform(const float *in1, float in2[3][4], float *out);
void ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);
void MatrixCopy(float in[3][4], float out[3][4]);
void QuaternionMatrix(vec4_t quaternion, float (*matrix)[4]);
void QuaternionSlerp(vec4_t p, vec4_t q, float t, vec4_t qt);
void AngleQuaternion(float *angles, vec4_t quaternion);
/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
// mathlib.h

#pragma once

#include "mathlib/mathlib.h"

typedef vec_t vec4_t[4]; // x,y,z,w
typedef vec_t vec5_t[5];

typedef short vec_s_t;
typedef vec_s_t vec3s_t[3];
typedef vec_s_t vec4s_t[4]; // x,y,z,w
typedef vec_s_t vec5s_t[5];

typedef int fixed4_t;
typedef int fixed8_t;
typedef int fixed16_t;

struct mplane_s;

float Length(const float* v);
void VectorInverse(float* v);

void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);

fixed16_t Invert24To16(fixed16_t val);

void AngleVectors(const Vector& angles, Vector* forward, Vector* right, Vector* up);
void AngleVectorsTranspose(const Vector& angles, Vector* forward, Vector* right, Vector* up);
#define AngleIVectors AngleVectorsTranspose

void AngleMatrix(const float* angles, float (*matrix)[4]);
void AngleIMatrix(const Vector& angles, float (*matrix)[4]);
void VectorTransform(const float* in1, float in2[3][4], float* out);

void NormalizeAngles(float* angles);
void InterpolateAngles(float* start, float* end, float* output, float frac);
float AngleBetweenVectors(const float* v1, const float* v2);

void VectorMatrix(const Vector& forward, Vector& right, Vector& up);
void VectorAngles(const float* forward, float* angles);

int InvertMatrix(const float* m, float* out);

// Here are some "manual" INLINE routines for doing floating point to integer conversions
extern short new_cw, old_cw;

typedef union DLONG
{
	int i[2];
	double d;
	float f;
} DLONG;

extern DLONG dlong;

// wtf is even going on here??
#ifdef _WIN32
	void __inline set_fpu_cw(void)
	{
		_asm
		{		wait
			fnstcw	old_cw
			wait
			mov		ax, word ptr old_cw
			or		ah, 0xc
			mov		word ptr new_cw,ax
			fldcw	new_cw
		}
	}

	int __inline quick_ftol(float f)
	{
		_asm {
			// Assumes that we are already in chop mode, and only need a 32-bit int
		fld		DWORD PTR f
		fistp	DWORD PTR dlong
		}
		return dlong.i[0];
	}

	void __inline restore_fpu_cw(void)
	{
		_asm fldcw old_cw
	}
#else
#define set_fpu_cw() /* */
#define quick_ftol(f) ftol(f)
#define restore_fpu_cw() /* */
#endif

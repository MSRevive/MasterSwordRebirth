/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DOUBLEVEC_T
typedef double vec_t;
#else
typedef float vec_t;
#endif
typedef vec_t Vector [3];	// x,y,z
typedef vec_t vec4_t[4];	// x,y,z,w

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#define	Q_PI	3.14159265358979323846

extern Vector  vec3_origin;

// Use this definition globally
#define	ON_EPSILON		0.01
#define	EQUAL_EPSILON	0.001

int VectorCompare (Vector  v1, Vector  v2);

#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorFill(a,b) { (a)[0]=(b); (a)[1]=(b); (a)[2]=(b);}
#define VectorAvg(a) ( ( (a)[0] + (a)[1] + (a)[2] ) / 3 )
#define VectorSubtract(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}
#define VectorAdd(a,b,c) {(c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];(c)[2]=(a)[2]+(b)[2];}
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
#define VectorScale(a,b,c) {(c)[0]=(b)*(a)[0];(c)[1]=(b)*(a)[1];(c)[2]=(b)*(a)[2];}

vec_t Q_rint (vec_t in);
vec_t _DotProduct (Vector  v1, Vector  v2);
void _VectorSubtract (Vector  va, Vector  vb, Vector  out);
void _VectorAdd (Vector  va, Vector  vb, Vector  out);
void _VectorCopy (Vector  in, Vector  out);
void _VectorScale (Vector  v, vec_t scale, Vector  out);

double VectorLength(Vector  v);

void VectorMA (Vector  va, double scale, Vector  vb, Vector  vc);

void CrossProduct (Vector  v1, Vector  v2, Vector  cross);
vec_t VectorNormalize (Vector  v);
void VectorInverse (Vector  v);

void ClearBounds (Vector  mins, Vector  maxs);
void AddPointToBounds (Vector  v, Vector  mins, Vector  maxs);

void AngleMatrix (const Vector  angles, float matrix[3][4] );
void AngleIMatrix (const Vector  angles, float matrix[3][4] );
void R_ConcatTransforms (const float in1[3][4], const float in2[3][4], float out[3][4]);

void VectorIRotate (const Vector  in1, const float in2[3][4], Vector  out);
void VectorRotate (const Vector  in1, const float in2[3][4], Vector  out);

void VectorTransform (const Vector  in1, const float in2[3][4], Vector  out);

void AngleQuaternion( const Vector  angles, vec4_t quaternion );
void QuaternionMatrix( const vec4_t quaternion, float (*matrix)[4] );
void QuaternionSlerp( const vec4_t p, vec4_t q, float t, vec4_t qt );


#ifdef __cplusplus
}
#endif

#endif

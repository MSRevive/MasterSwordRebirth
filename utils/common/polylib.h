/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/


typedef struct
{
	int		numpoints;
	Vector 	p[8];		// variable sized
} winding_t;

#define	MAX_POINTS_ON_WINDING	128

winding_t	*AllocWinding (int points);
vec_t	WindingArea (winding_t *w);
void	WindingCenter (winding_t *w, Vector  center);
void	ClipWinding (winding_t *in, Vector  normal, vec_t dist,
					 winding_t **front, winding_t **back);
void	ClipWindingNoCopy (winding_t *in, Vector  normal, vec_t dist,
					 winding_t **front, winding_t **back);
winding_t	*ChopWinding (winding_t *in, Vector  normal, vec_t dist);
winding_t	*ChopWindingNoFree (winding_t *in, Vector  normal, vec_t dist);
winding_t	*CopyWinding (winding_t *w);
winding_t	*BaseWindingForPlane (Vector  normal, float dist);
void	CheckWinding (winding_t *w);
void	WindingPlane (winding_t *w, Vector  normal, vec_t *dist);
void	RemoveColinearPoints (winding_t *w);
int		WindingOnPlaneSide (winding_t *w, Vector  normal, vec_t dist);
void	FreeWinding (winding_t *w);
void	WindingBounds (winding_t *w, Vector  mins, Vector  maxs);
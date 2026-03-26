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

//
// pm_shared.h
//
#if !defined(PM_SHAREDH)
#define PM_SHAREDH
#pragma once

constexpr float STOP_EPSILON = 0.1;
constexpr float DIST_EPSILON = 0.125f;	// Max error from network coordinate quantization
constexpr unsigned int MAX_CTEXTURES = 512;	// max number of textures loaded

void PM_Init(struct playermove_s *ppmove);
void PM_Move(struct playermove_s *ppmove, qboolean server);
char PM_FindTextureType(char *name);
constexpr float PM_CHECKSTUCK_MINTIME = 0.05; // Don't check again too quickly.

/**
*	@brief Engine calls this to enumerate player collision hulls, for prediction. Return false if the hullnumber doesn't exist.
*/
bool PM_GetHullBounds(int hullnumber, float* mins, float* maxs);

char* memfgets(const byte* pMemFile, std::size_t fileSize, std::size_t& filePos, char* pBuffer, std::size_t bufferSize);

// Spectator Movement modes (stored in pev->iuser1, so the physics code can get at them)
enum obs_type_e {

	OBS_NONE = 0,
	OBS_CHASE_LOCKED = 1,
	OBS_CHASE_FREE = 2,
	OBS_ROAMING = 3,
	OBS_IN_EYE = 4,
	OBS_MAP_FREE = 5,
	OBS_MAP_CHASE = 6

};

inline bool g_CheckForPlayerStuck = false;

enum step_surface_e {

	STEP_CONCRETE = 0, // default step sound
	STEP_METAL = 1,	// metal floor
	STEP_DIRT = 2,		// dirt, sand, rock
	STEP_VENT = 3,		// ventillation duct
	STEP_GRATE = 4,	// metal grating
	STEP_TILE = 5,		// floor tiles
	STEP_SLOSH = 6,	// shallow liquid puddle
	STEP_WADE = 7,		// wading in liquid
	STEP_LADDER = 8,	// climbing ladder
	STEP_SNOW = 9		// MAR2008a snow

};




#endif

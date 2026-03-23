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

constexpr unsigned int MAX_CDPVPRESET = 27;

void PM_Init(struct playermove_s *ppmove);
void PM_Move(struct playermove_s *ppmove, qboolean server);
char PM_FindTextureType(char *name);

/**
*	@brief Engine calls this to enumerate player collision hulls, for prediction. Return false if the hullnumber doesn't exist.
*/
bool PM_GetHullBounds(int hullnumber, float* mins, float* maxs);

char* memfgets(const byte* pMemFile, std::size_t fileSize, std::size_t& filePos, char* pBuffer, std::size_t bufferSize);

// Spectator Movement modes (stored in pev->iuser1, so the physics code can get at them)
enum {

	OBS_NONE = 0,
	OBS_CHASE_LOCKED = 1,
	OBS_CHASE_FREE = 2,
	OBS_ROAMING = 3,
	OBS_IN_EYE = 4,
	OBS_MAP_FREE = 5,
	OBS_MAP_CHASE = 6

};

inline bool g_CheckForPlayerStuck = false;

#endif

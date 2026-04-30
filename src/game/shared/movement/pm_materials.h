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
#if !defined(PM_MATERIALSH)
#define PM_MATERIALSH
#pragma once

inline constexpr unsigned int MAX_CBTEXTURENAME = 13; // only load first n chars of name

enum char_texture_e {
	CHAR_TEX_CONCRETE = 'C', // texture types
	CHAR_TEX_METAL = 'M',
	CHAR_TEX_DIRT = 'D',
	CHAR_TEX_VENT = 'V',
	CHAR_TEX_GRATE = 'G',
	CHAR_TEX_TILE = 'T',
	CHAR_TEX_SLOSH = 'S',
	CHAR_TEX_WOOD = 'W',
	CHAR_TEX_COMPUTER = 'P',
	CHAR_TEX_GLASS = 'Y',
	CHAR_TEX_FLESH = 'F',
	CHAR_TEX_SNOW = 'N'

};

#endif // !PM_MATERIALSH

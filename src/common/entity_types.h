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
// entity_types.h
#if !defined(ENTITY_TYPESH)
#define ENTITY_TYPESH

constexpr int ET_NORMAL = 0;
constexpr int ET_PLAYER = 1;
constexpr int ET_TEMPENTITY = 2;
constexpr int ET_BEAM = 3;
// BMODEL or SPRITE that was split across BSP nodes
constexpr int ET_FRAGMENTED = 4;

#endif // !ENTITY_TYPESH

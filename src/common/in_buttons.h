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
#ifndef IN_BUTTONS_H
#define IN_BUTTONS_H
#ifdef _WIN32
#pragma once
#endif

constexpr int IN_ATTACK = (1 << 0);
constexpr int IN_JUMP = (1 << 1);
constexpr int IN_DUCK = (1 << 2);
constexpr int IN_FORWARD = (1 << 3);
constexpr int IN_BACK = (1 << 4);
constexpr int IN_USE = (1 << 5);
constexpr int IN_CANCEL = (1 << 6);
constexpr int IN_LEFT = (1 << 7);
constexpr int IN_RIGHT = (1 << 8);
constexpr int IN_MOVELEFT = (1 << 9);
constexpr int IN_MOVERIGHT = (1 << 10);
constexpr int IN_ATTACK2 = (1 << 11);
constexpr int IN_RUN = (1 << 12);
constexpr int IN_RELOAD = (1 << 13);
constexpr int IN_ALT1 = (1 << 14);
constexpr int IN_SCORE = (1 << 15);// Used by client.dll for when scoreboard is held down

#endif // IN_BUTTONS_H

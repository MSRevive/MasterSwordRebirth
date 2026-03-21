/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
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
#ifndef DOORS_H
#define DOORS_H
#endif

// doors
enum {
	SF_DOOR_ROTATE_Y = 0,
	SF_DOOR_START_OPEN = 1,
	SF_DOOR_ROTATE_BACKWARDS = 2,
	SF_DOOR_PASSABLE =  8,
	SF_DOOR_ONEWAY = 16,
	SF_DOOR_NO_AUTO_RETURN = 32,
	SF_DOOR_ROTATE_Z = 64,
	SF_DOOR_ROTATE_X = 128,
	SF_DOOR_USE_ONLY = 256,    // door must be opened by player's use button.
	SF_ITEM_USE_ONLY = 256, //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!!
	SF_BUTTON_USE_ONLY = 256,
	SF_DOOR_NOMONSTERS = 512 ,  // Monster can't open
	SF_DOOR_UNBLOCKABLE = 1024, //Thothie AUG2011_29 - make door unstoppable
	SF_DOOR_SILENT = 0x80000000,
	SF_PLAYERCLIP_START_OFF = 0x0001,
	SF_WALL_START_OFF = 0x0001,
	SF_MONSTERCLIP_START_OFF = 0x0001,
	SF_PENDULUM_SWING = 2, // spawnflag that makes a pendulum a rope swing.
	SF_BRUSH_ACCDCC = 16,		 // brush should accelerate and decelerate when toggled
	SF_BRUSH_HURT = 32,		 // rotating brush that inflicts pain based on rotation speed
	SF_ROTATING_NOT_SOLID = 64, // some special rotating objects are not solid.
	SF_CONVEYOR_VISUAL = 0x0001,
	SF_CONVEYOR_NOTSOLID = 0x0002,
	SF_CONVEYOR_TOGGLE = 0x0004, //Thothie AUG2011_30 toggle conveyors
	SF_NORESPAWN = (1 << 30) // !!!set this bit on guns and stuff that should never respawn.
};


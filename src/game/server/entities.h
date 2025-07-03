/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// entities.h
//=========================================================

struct entity_t
{
	char classname[32];
	float angles[3];
	float origin[3];
};

//=========================================================
// CEntity 
//=========================================================

class CEntity
{
public:
	void	InitEntity( void );
	
	int		CheckENTFile(char *szMapName);
	int		FLoadEntity(char *szMapName);
private:
	void	FixKeyName( char *key );
	char *aMemFile;
	char *pMemFile;
	int length;
};


extern CEntity WorldEntity;

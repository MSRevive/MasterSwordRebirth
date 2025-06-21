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
// entities.cpp - Extra entity stuff.
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"entities.h"

extern char *COM_Parse (char *data);
extern char com_token[];

CEntity	WorldEntity;

//=========================================================
// CEntity - InitEntity - prepare for use.
//=========================================================
void CEntity::InitEntity( void)
{
	pMemFile = NULL;
	aMemFile = NULL;
}

void CEntity::FixKeyName( char *key )
{
	// Fix keynames with trailing spaces
	int n = strlen( key );
	while( n && key[n - 1] == ' ' )
	{
		key[n - 1] = 0;
		n--;
	}
}

//=========================================================
// CGraph - FLoadGraph - attempts to load a node graph from disk.
// if the current level is maps/snar.bsp, maps/graphs/snar.nod
// will be loaded. If file cannot be loaded, the node tree
// will be created and saved to disk.
//=========================================================
int CEntity::FLoadEntity ( char *szMapName )
{
	int retValue = TRUE;
	int	error = 0;

	entity_t ent;

	// maximums from .rmf format
	char szKey[32];

	// Didn't find opening brace?
	if( pMemFile[0] != '{' )
	{
		ALERT( at_console, "CEntity::FLoadEntity: found %c when expecting {\n", pMemFile[0] );

		retValue = FALSE;
		goto fail;
	}

	// parse entities from an .ent file
	while( 1 )
	{
		// parse the opening brace	
		pMemFile = COM_Parse( pMemFile );

		// no more file
		if( !pMemFile )
		{
			break;
		}

		// parse an entity
		while( 1 )
		{	
			// Parse key
			pMemFile = COM_Parse( pMemFile );

			// End of the entity
			if( com_token[0] == '}' )
				break;
			
			// Ran out of input buffer?
			if( !pMemFile )
			{
				error = 1;
				break;
			}
			
			// Store off the key
			strcpy( szKey, com_token );

			FixKeyName( szKey );

			// Parse value	
			pMemFile = COM_Parse( pMemFile );

			// Ran out of buffer?
			if( !pMemFile )
			{
				error = 1;
				break;
			}

			// Hit the end instead of a value?
			if( com_token[0] == '}' )
			{
				error = 1;
				break;
			}

			if( !strcmp( szKey, "classname" ) )
			{
				strcpy( ent.classname, com_token );
			}
			else if( !strcmp( szKey, "origin" ) )
			{
				int i = sscanf( com_token, "%f %f %f", &ent.origin[0], &ent.origin[1], &ent.origin[2] );

				if( i != 3 )
				{
					error = 1;
					break;
				}
			}
			else if( !strcmp( szKey, "angles" ) )
			{
				int i = sscanf( com_token, "%f %f %f", &ent.angles[0], &ent.angles[1], &ent.angles[2] );

				if( i != 3 )
				{
					error = 1;
					break;
				}
			}
		}

		if( strcmp( ent.classname, "worldspawn" ) )
		{
			CBaseEntity::Create( ent.classname, ent.origin, ent.angles );
		}
		else
		{
			ALERT( at_console, "CEntity::FLoadEntity: skipping worldspawn\n" );
		}


		// Check for errors and abort if any
		if( error )
		{
			ALERT( at_console, "CEntity::FLoadEntity: error parsing entities\n" );

			retValue = FALSE;
			break;
		}
	}

fail:
	FREE_FILE( aMemFile );

	return retValue;
}

//=========================================================
// CEntity - CheckENTFile - this function checks the 
// existence of the BSP file that was just loaded.
//
// returns FALSE if the .ENT file doesn't exist.
//=========================================================
int CEntity::CheckENTFile ( char *szMapName )
{
	int 		retValue;

	char		szEntityFilename[MAX_PATH];
	

	strcpy ( szEntityFilename, "maps/" );
	strcat ( szEntityFilename, szMapName );
	strcat ( szEntityFilename, ".ent" );
	
	retValue = TRUE;

	pMemFile = aMemFile = (char *)LOAD_FILE_FOR_ME( szEntityFilename, &length );

	if( !aMemFile )
	{
		ALERT ( at_aiconsole, ".ENT File not found\n\n" );
		retValue = FALSE;
	}
	else
	{
		retValue = TRUE;
	}

	return retValue;
}
